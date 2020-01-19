/*
 * Copyright (c) 2018 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA. 
 *
 * $Id: //eng/uds-releases/flanders/src/uds/context.c#9 $
 */

#include "context.h"

#include "errors.h"
#include "featureDefs.h"
#include "grid.h"
#include "hashUtils.h"
#include "indexSession.h"
#include "isCallbackThreadDefs.h"
#include "logger.h"
#include "memoryAlloc.h"
#include "parameter.h"
#include "permassert.h"
#include "requestLimit.h"
#include "threads.h"
#include "timeUtils.h"
#include "udsState.h"

enum {
  DEFAULT_REQUEST_LIMIT = 1024,
  MAX_REQUEST_LIMIT     = 2048
};

/**********************************************************************/
static UdsParameterValue getDefaultTurnaroundEnabled(void)
{
  UdsParameterValue value;

#if ENVIRONMENT
  const char *env = getenv(UDS_TIME_REQUEST_TURNAROUND);
  if (env != NULL) {
    UdsParameterValue tmp = {
      .type = UDS_PARAM_TYPE_STRING,
      .value.u_string = env,
    };
    if (validateBoolean(&tmp, NULL, &value) == UDS_SUCCESS) {
      return value;
    }
  }
#endif // ENVIRONMENT

  value.type = UDS_PARAM_TYPE_BOOL;
  value.value.u_bool = false;
  return value;
}

/**********************************************************************/
int defineTimeRequestTurnaround(ParameterDefinition *pd)
{
  pd->validate     = validateBoolean;
  pd->currentValue = getDefaultTurnaroundEnabled();
  pd->update       = NULL;
  return UDS_SUCCESS;
}

/**********************************************************************/
static void handleCallbacks(Request *request)
{
  if (request->isControlMessage) {
    request->status = dispatchContextControlRequest(request);
    /*
     * This is a synchronous control request for collecting or resetting the
     * context statistics, so we use enterCallbackStage() to return the
     * request to the client thread even though this is the callback thread.
     */
    enterCallbackStage(request);
    return;
  }

  if (request->status == UDS_SUCCESS) {
    // Measure the turnaround time of this request and include that time,
    // along with the rest of the request, in the context's StatCounters.
    updateRequestContextStats(request);
  }

  if (request->callback != NULL) {
    // The request has specified its own callback and does not expect to be
    // freed, but free the serverContext that's hidden from the client.
    FREE(request->serverContext);
    request->serverContext = NULL;
    UdsContext *context = request->context;
    request->found = (request->location != LOC_UNAVAILABLE);
    request->callback((UdsRequest *) request);
    releaseBaseContext(context);
    return;
  }

  if (request->context->hasCallback) {
    UdsBlockAddress duplicateAddress = NULL;
    if ((request->type != UDS_DELETE)) {
      duplicateAddress = &request->newMetadata.data;
    }
    UdsBlockAddress canonicalAddress = NULL;
    if (request->location != LOC_UNAVAILABLE) {
      canonicalAddress = &request->oldMetadata.data;
    }
    UdsContext *context = request->context;
    UdsBlockContext blockContext = { .id = context->id };
    // Allow the callback routine to create a new request if necessary without
    // blocking our thread. "request" is just a handy non-null value here.
    setCallbackThread(request);
    context->callbackFunction(blockContext, request->type, request->status,
                              request->cookie, duplicateAddress,
                              canonicalAddress, &request->hash,
                              context->callbackArgument);
    setCallbackThread(NULL);
  }

  freeRequest(request);
}

/**********************************************************************/
static int validateMetadataSize(unsigned int metadataSize)
{
  if (metadataSize <= UDS_MAX_BLOCK_DATA_SIZE) {
    return UDS_SUCCESS;
  }
  return logWarningWithStringError(UDS_INVALID_METADATA_SIZE,
                                   "invalid metadata size: %u", metadataSize);
}

/**********************************************************************/
int openContext(UdsIndexSession  session,
                unsigned int     metadataSize,
                unsigned int    *contextID)
{
  int result = validateMetadataSize(metadataSize);
  if (result != UDS_SUCCESS) {
    return result;
  }

  udsInitialize();

  lockGlobalStateMutex();
  result = checkLibraryRunning();
  if (result != UDS_SUCCESS) {
    unlockGlobalStateMutex();
    return result;
  }

  // Hold a session group reference until the context is added, or fails.
  SessionGroup *contextGroup = getContextGroup();
  result = acquireSessionGroup(contextGroup);
  if (result != UDS_SUCCESS) {
    unlockGlobalStateMutex();
    return result;
  }

  // This index session reference is kept by the context object
  // and only released for an error return.
  IndexSession *indexSession;
  result = getIndexSession(session.id, &indexSession);
  if (result != UDS_SUCCESS) {
    releaseSessionGroup(contextGroup);
    unlockGlobalStateMutex();
    return result;
  }

  UdsContext *context = NULL;
  result = makeBaseContext(indexSession, metadataSize, &context);
  if (result != UDS_SUCCESS) {
    releaseSessionGroup(contextGroup);
    releaseIndexSession(indexSession);
    unlockGlobalStateMutex();
    return result;
  }
  // The non-null context now owns the indexSession reference.

  // Publish the new context in the context SessionGroup.
  *contextID = initializeSession(contextGroup,
                                 &context->session,
                                 (SessionContents) context);
  context->id = *contextID;
  logDebug("Opened context (%u)", *contextID);
  releaseBaseContext(context);
  releaseSessionGroup(contextGroup);
  unlockGlobalStateMutex();
  return UDS_SUCCESS;
}

/**********************************************************************/
static int checkContext(UdsContext *context)
{
  switch (context->contextState) {
    case UDS_CS_READY:
      return checkIndexSession(context->indexSession);
    case UDS_CS_DISABLED:
      return UDS_DISABLED;
    default:
      return UDS_NOCONTEXT;
  }
}

/**********************************************************************/
int getBaseContext(unsigned int contextId, UdsContext **contextPtr)
{
  Session *session;
  int result = getSession(getContextGroup(), contextId, &session);
  if (result != UDS_SUCCESS) {
    return result;
  }

  UdsContext *context = (UdsContext *) getSessionContents(session);
  result = checkContext(context);
  if (result != UDS_SUCCESS) {
    releaseSession(session);
    return result;
  }

  *contextPtr = context;
  return result;
}

/**********************************************************************/
void releaseBaseContext(UdsContext *context)
{
  releaseSession(&context->session);
}

/**********************************************************************/
int handleError(UdsContext *context, int errorCode)
{
  if (isUnrecoverable(errorCode)) {
    if (context != NULL) {
      context->contextState = UDS_CS_DISABLED;
      if (context->indexSession != NULL) {
        setIndexSessionState(context->indexSession, IS_DISABLED);
      }
    }
  }

  // Make sure the client never sees our internal code or attributes
  return sansUnrecoverable(errorCode);
}

/**********************************************************************/
int handleErrorAndReleaseBaseContext(UdsContext *context, int errorCode)
{
  int result = handleError(context, errorCode);
  releaseBaseContext(context);
  return sansUnrecoverable(result);
}

/**********************************************************************/
void freeContext(UdsContext *context)
{
  if (context == NULL) {
    return;
  }
  requestQueueFinish(context->callbackQueue);
  context->callbackQueue = NULL;

  if (context->indexSession != NULL) {
    releaseIndexSession(context->indexSession);
  }

  freeRequestLimit(context->requestLimit);
  FREE(context);
}

/**********************************************************************/
int closeContext(unsigned int contextId)
{
  SessionGroup *contextGroup = getContextGroup();
  int result = acquireSessionGroup(contextGroup);
  if (result != UDS_SUCCESS) {
    return result;
  }

  Session *session;
  result = getSession(contextGroup, contextId, &session);
  if (result != UDS_SUCCESS) {
    releaseSessionGroup(contextGroup);
    return result;
  }

  UdsContext *context = (UdsContext *) getSessionContents(session);

  finishSession(contextGroup, &context->session);
  logDebug("Closed context (%u)", contextId);

  freeContext(context);
  releaseSessionGroup(contextGroup);
  return UDS_SUCCESS;
}

/**********************************************************************/
int makeBaseContext(IndexSession  *indexSession,
                    unsigned int   metadataSize,
                    UdsContext   **contextPtr)
{
  UdsContext *context;
  int result = ALLOCATE(1, UdsContext, "empty context", &context);
  if (result != UDS_SUCCESS) {
    return result;
  }

  context->indexSession       = indexSession;
  context->contextState       = UDS_CS_READY;
  context->metadataSize       = metadataSize;

  // Make sure we've captured any environment override of
  // timeRequestTurnaround before assigning it to the context.

  UdsParameterValue value;
  if ((udsGetParameter(UDS_TIME_REQUEST_TURNAROUND, &value) == UDS_SUCCESS)
      && (value.type == UDS_PARAM_TYPE_BOOL)) {
    context->timeRequestTurnaround = value.value.u_bool;
  } else {
    context->timeRequestTurnaround = false;
  }

  result = makeRequestLimit(DEFAULT_REQUEST_LIMIT, &context->requestLimit);
  if (result != UDS_SUCCESS) {
    freeContext(context);
    return result;
  }

  result = makeRequestQueue("callbackW", &handleCallbacks,
                            &context->callbackQueue);
  if (result != UDS_SUCCESS) {
    freeContext(context);
    return result;
  }

  context->stats.resetTime = asTimeT(currentTime(CT_REALTIME));
  *contextPtr = context;
  return UDS_SUCCESS;
}

/**********************************************************************/
void flushBaseContext(UdsContext *context)
{
  waitForIdleSession(&context->session);
}

/**********************************************************************/
int flushContext(unsigned int contextId)
{
  UdsContext *context;
  int result = getBaseContext(contextId, &context);
  if (result != UDS_SUCCESS) {
    return result;
  }

  flushBaseContext(context);
  releaseBaseContext(context);
  return UDS_SUCCESS;
}

/**********************************************************************/
int getConfiguration(unsigned int contextId, UdsConfiguration *userConfig)
{
  if (userConfig == NULL) {
    return logErrorWithStringError(UDS_CONF_PTR_REQUIRED,
                                   "received a NULL config pointer");
  }

  UdsContext *context;
  int result = getBaseContext(contextId, &context);
  if (result != UDS_SUCCESS) {
    return logErrorWithStringError(result, "getBaseContext() failed");
  }

  result = ALLOCATE(1, struct udsConfiguration, "udsConfiguration",
                    userConfig);
  if (result != UDS_SUCCESS) {
    result = logErrorWithStringError(result,
                                     "allocation of udsConfiguration failed.");
  } else {
    **userConfig = context->indexSession->grid->userConfig;
  }
  return handleErrorAndReleaseBaseContext(context, result);
}

/**
 * Adjust the number of outstanding requests permitted for this
 * context; external version (aside from the per-context-type
 * wrapping) with range checks and locking.
 **/
int setRequestQueueLimit(unsigned int contextId, unsigned int maxRequests)
{
  if (maxRequests > MAX_REQUEST_LIMIT) {
    return logWarningWithStringError(
      UDS_REQUESTS_OUT_OF_RANGE,
      "attempt to set very large limit on request queue");
  } else if (maxRequests == 0) {
    return logWarningWithStringError(
      UDS_REQUESTS_OUT_OF_RANGE, "attempt to set request queue limit to zero");
  }

  UdsContext *context;
  int result = getBaseContext(contextId, &context);
  if (result != UDS_SUCCESS) {
    return result;
  }

  setRequestPermitLimit(context->requestLimit, maxRequests);

  releaseBaseContext(context);
  return UDS_SUCCESS;
}

/**********************************************************************/
int getContextIndexStats(unsigned int contextId, UdsIndexStats *stats)
{
  if (stats == NULL) {
    return logErrorWithStringError(UDS_INDEX_STATS_PTR_REQUIRED,
                                   "received a NULL index stats pointer");
  }

  UdsContext *context;
  int result = getBaseContext(contextId, &context);
  if (result != UDS_SUCCESS) {
    return logErrorWithStringError(result, "getBaseContext() failed.");
  }

  IndexRouterStatCounters routerStats;
  result = getGridStatistics(context->indexSession->grid, &routerStats);
  if (result != UDS_SUCCESS) {
    return handleErrorAndReleaseBaseContext(context, result);
  }

  stats->entriesIndexed   = routerStats.entriesIndexed;
  stats->memoryUsed       = routerStats.memoryUsed;
  stats->diskUsed         = routerStats.diskUsed;
  stats->collisions       = routerStats.collisions;
  stats->entriesDiscarded = routerStats.entriesDiscarded;
  stats->checkpoints      = routerStats.checkpoints;

  return handleErrorAndReleaseBaseContext(context, result);
}

/**********************************************************************/
int getContextStats(unsigned int contextId, UdsContextStats *stats)
{
  if (stats == NULL) {
    return
      logWarningWithStringError(UDS_CONTEXT_STATS_PTR_REQUIRED,
                                "received a NULL context stats pointer");
  }

  UdsContext *context;
  int         result = getBaseContext(contextId, &context);
  if (result != UDS_SUCCESS) {
    return logWarningWithStringError(result, "getBaseContext() failed");
  }

  // Send a synchronous control message to the callback thread to safely
  // gather the context statistics.
  Request *request;
  result = launchClientControlMessage(context, stats,
                                      REQUEST_COLLECT_CONTEXT_STATS,
                                      STAGE_CALLBACK, &request);
  if (result != UDS_SUCCESS) {
    return handleErrorAndReleaseBaseContext(context, result);
  }

  // The request directly updated stats, so there's no need to copy anything
  // else out of the request.
  result = request->status;
  freeRequest(request);
  return handleError(context, result);
}

/**********************************************************************/
static void collectStats(const UdsContext *context, UdsContextStats *stats)
{
  const StatCounters *counters = &context->stats.counters;

  stats->resetTime = context->stats.resetTime;
  stats->currentTime = asTimeT(currentTime(CT_REALTIME));

  stats->postsFound           = counters->postsFound;
  stats->inMemoryPostsFound   = counters->postsFoundOpenChapter;
  stats->densePostsFound      = counters->postsFoundDense;
  stats->sparsePostsFound     = counters->postsFoundSparse;
  stats->postsNotFound        = counters->postsNotFound;
  stats->updatesFound         = counters->updatesFound;
  stats->updatesNotFound      = counters->updatesNotFound;
  stats->deletionsFound       = counters->deletionsFound;
  stats->deletionsNotFound    = counters->deletionsNotFound;
  stats->queriesFound         = counters->queriesFound;
  stats->queriesNotFound      = counters->queriesNotFound;
  stats->requests             = counters->requests;
  stats->requestTurnaroundTime = counters->requestTurnaroundTime;
  stats->maximumTurnaroundTime = counters->maximumTurnaroundTime;

  /*
   * Strictly speaking, this is a context configuration parameter,
   * not a measured statistic.  But we don't have any place to put
   * per-context configuration data, and it's probably not worth
   * adding it just for this one field.
   */
  stats->requestQueueLimit = getRequestPermitLimit(context->requestLimit);
}

/**********************************************************************/
int resetStats(unsigned int contextId)
{
  UdsContext  *context;
  int          result = getBaseContext(contextId, &context);
  if (result != UDS_SUCCESS) {
    return result;
  }

  // Send a synchronous control message to the callback thread to safely reset
  // the context statistics.
  Request *request;
  result = launchClientControlMessage(context, NULL,
                                      REQUEST_RESET_CONTEXT_STATS,
                                      STAGE_CALLBACK, &request);
  if (result != UDS_SUCCESS) {
    return handleErrorAndReleaseBaseContext(context, result);
  }

  result = request->status;
  freeRequest(request);
  return handleError(context, result);
}

/**********************************************************************/
int registerDedupeCallback(unsigned int            contextID,
                           UdsDedupeBlockCallback  callbackFunction,
                           void                   *callbackArgument)
{
  UdsContext *context;
  int result = getBaseContext(contextID, &context);
  if (result != UDS_SUCCESS) {
    return result;
  }

  if (callbackFunction == NULL) {
    context->hasCallback = false;
  } else if (context->hasCallback) {
    result = UDS_CALLBACK_ALREADY_REGISTERED;
  } else {
    context->callbackFunction = callbackFunction;
    context->callbackArgument = callbackArgument;
    context->hasCallback      = true;
  }

  releaseBaseContext(context);
  return result;
}

/**********************************************************************/
int dispatchContextControlRequest(Request *request)
{
  ContextStats *stats = &request->context->stats;
  switch (request->action) {
  case REQUEST_COLLECT_CONTEXT_STATS:
    collectStats(request->context, (UdsContextStats *) request->controlData);
    return UDS_SUCCESS;

  case REQUEST_RESET_CONTEXT_STATS:
    memset(&stats->counters, 0, sizeof(stats->counters));
    stats->resetTime = asTimeT(currentTime(CT_REALTIME));
    return UDS_SUCCESS;

  default:
    return ASSERT_FALSE("unsupported context control action %d",
                        request->action);
  }
}
