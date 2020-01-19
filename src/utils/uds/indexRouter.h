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
 * $Id: //eng/uds-releases/flanders-rhel7.5/src/uds/indexRouter.h#1 $
 */

#ifndef INDEX_ROUTER_H
#define INDEX_ROUTER_H

#include "compiler.h"
#include "indexRouterStats.h"
#include "request.h"

/**
 * Router types
 **/
typedef enum {
  ROUTER_LOCAL,
  ROUTER_REMOTE
} RouterType;

/**
 * Callback after a query, update or remove request completes and fills in
 * select fields in the request: status for all requests, oldMetadata and
 * hashExists for query and update requests.
 *
 * @param request     request object.
 **/
typedef void (*IndexRouterCallback)(Request *request);

/**
 * Forward declaration of all the index router function hooks. The struct is
 * declared lower down in this file due to its length.
 **/
typedef struct indexRouterMethods IndexRouterMethods;

/**
 * The header structure contain fields common to the all the index router
 * implementations.
 **/
struct indexRouter {
  RouterType                type;
  const IndexRouterMethods *methods;
  IndexRouterCallback       callback;
};

/**
 * Destroy an index router and free its memory.
 *
 * @param router  the index router to destroy (may be NULL)
 **/
void freeIndexRouter(IndexRouter *router);

/**
 * Index router methods as a function table in IndexRouter (see common.h).
 **/
struct indexRouterMethods {
  /**
   * Save the index router state to persistent storage.
   *
   * @param router  The index router to save.
   *
   * @return        UDS_SUCCESS if successful.
   **/
  int (*saveState)(IndexRouter *router);

  /**
   * Destroy the index router and free its memory.
   *
   * @param router The index router to destroy.
   **/
  void (*free)(IndexRouter *router);

  /**
   * Select and return the request queue responsible for executing the next
   * index stage of a request, updating the request with any associated state
   * (such as the zone number for UDS requests on a local index).
   *
   * @param router      The index router.
   * @param request     The Request destined for the queue.
   * @param nextStage   The next request stage (STAGE_TRIAGE or STAGE_INDEX).
   *
   * @return the next index stage queue (the local triage queue, local zone
   *         queue, or remote RPC send queue)
   **/
  RequestQueue *(*selectQueue)(IndexRouter  *router,
                               Request      *request,
                               RequestStage  nextStage);

  /**
   * Executes the index operation for a UDS request and calls the callback upon
   * completion.
   *
   * @param router      The index router.
   * @param request     A pointer to the Request to process.
   **/
  void (*execute)(IndexRouter *router, Request *request);

  /**
   * Gather router usage counters.
   *
   * @param router    The index router.
   * @param counters  The statistics structure to fill.
   *
   * @return          UDS_SUCCESS or error code
   **/
  int (*getStatistics)(IndexRouter *router, IndexRouterStatCounters *counters);

  /**
   * Change the checkpoint frequency.
   *
   * @param router    The index router.
   * @param frequency The new checkpointing frequency.
   **/
  void (*setCheckpointFrequency)(IndexRouter *router, unsigned int frequency);
};

/**
 * Check whether an index router is local.
 *
 * @param router The index router to check
 *
 * @return <code>true</code> if the router is local
 **/
static INLINE bool isRouterLocal(IndexRouter *router)
{
  return (router->type == ROUTER_LOCAL);
}

#endif /* INDEX_ROUTER_H */
