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
 * $Id: //eng/uds-releases/flanders-rhel7.5/src/uds/context.h#1 $
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include "common.h"
#include "cpu.h"
#include "featureDefs.h"
#include "namespaceHash.h"
#include "opaqueTypes.h"
#include "session.h"
#include "uds-block.h"
#include "util/atomic.h"

/**
 * The current context state.
 **/
typedef enum udsContextState {
  UDS_CS_READY     = 1,
  UDS_CS_DISABLED  = 2
} UdsContextState;

/**
 * valid context types
 **/
typedef enum {
  BLOCK_CONTEXT
} ContextType;

typedef struct statCounters {
  uint64_t postsFound;          /* Post calls that found an entry */
  uint64_t postsFoundOpenChapter; /* Post calls found in the open chapter */
  uint64_t postsFoundDense;     /* Post calls found in the dense index */
  uint64_t postsFoundSparse;    /* Post calls found in the sparse index */
  uint64_t postsNotFound;       /* Post calls that did not find an entry */
  uint64_t postsFoundData;      /* Post calls that found an entry (w/data) */
  uint64_t postsNotFoundData;   /* Post calls that did not find an entry (w/data) */
  uint64_t updatesFound;        /* Update calls that found an entry */
  uint64_t updatesNotFound;     /* Update calls that did not find an entry */
  uint64_t deletionsFound;      /* Delete calls that found an entry */
  uint64_t deletionsNotFound;   /* Delete calls that did not find an entry */
  uint64_t queriesFound;        /* Query calls that found an entry */
  uint64_t queriesNotFound;     /* Query calls that did not find an entry */
  uint64_t requests;            /* Total number of requests */
  uint64_t bytesFound;          /* Bytes found */
  uint64_t bytesNotFound;       /* Bytes not found */
  int64_t  requestTurnaroundTime; /* Total turnaround (in us) of requests */
  int64_t  maximumTurnaroundTime; /* Maximum turnaround (in us) of requests */
} StatCounters;

typedef struct __attribute__((aligned(CACHE_LINE_BYTES))) contextStats {
  time_t          resetTime;
  StatCounters    counters;
} ContextStats;

typedef union {
  UdsDedupeBlockCallback  blockCallback;
} IndexCallbackFunction;

/**
 * A function to encode metadata for an index request.
 *
 * @param metadata The metadata to encode
 * @param request  The request in which to encode the metadata
 **/
typedef void (*MetadataEncoder)(const void *metadata, Request *request);

/**
 * Signature for functions which takes a completed request and passes it to
 * the user's registered dedupe callback.
 *
 * @param request The completed request
 **/
typedef void (*CallbackHandler)(Request *request);

/**
 * Context for uds client index requests.
 **/
typedef struct  __attribute__((aligned(CACHE_LINE_BYTES))) udsContext {
  /* The id of this context */
  unsigned int             id;
  /* The type of context (block, file, or stream) */
  ContextType              type;
  /* The state of this context (whether or not it may be used) */
  UdsContextState          contextState;

  /* The index and session which own this context */
  IndexSession            *indexSession;
  Session                  session;

#if NAMESPACES
  /* The hash of the namespace for this context */
  NamespaceHash            namespaceHash;
#endif /* NAMESPACES */
  /* Application metadata size; the library may store more! */
  unsigned int             metadataSize;
  /**
   * Encoder to provide context type specific metadata encoding into index
   * requests
   **/
  MetadataEncoder          metadataEncoder;
  /* Chunk size for modes that need it  */
  unsigned int             chunkSize;

  /* true if turnaround time should be measured on requests to this context */
  bool                     timeRequestTurnaround;

  /* Callback for index requests */
  bool                     hasCallback;
  CallbackHandler          callbackHandler;
  IndexCallbackFunction    callbackFunction;
  void                    *callbackArgument;
  RequestQueue            *callbackQueue;

  /** limit on the number of outstanding requests */
  RequestLimit            *requestLimit;

  /** request statistics for this context, all owned by the callback thread */
  ContextStats             stats;

  /** atomic counter for distributing requests to the hash queues */
  Atomic32                 hashQueueRotor;

  /** function to use for hashing requests */
  UdsChunkName           (*chunkNameGenerator)(const void *data, size_t size);
} UdsContext;

/**
 * Return a string literal describing the given context type.
 **/
const char *contextTypeToString(ContextType contextType);

#if NAMESPACES
/**
 * Open a context.
 *
 * @param [in]  session          The index session on which to open a context
 * @param [in]  namespace        The namespace to associate with the context
 * @param [in]  metadataSize     The application metadata size to copy
 * @param [in]  metadataEncoder  A function to encode application metadata
 *                               into an index request
 * @param [in]  chunkSize        The data chunk size, for file/stream contexts
 * @param [in]  contextType      The type of context to open
 * @param [in]  callbackHandler  The function that passes results in completed
 *                               requests back to the user's callback function
 * @param [out] contextID        A point to hold the id of the new context
 *
 * @return UDS_SUCCESS or an error
 **/
int openContext(UdsIndexSession     session,
                const UdsNamespace *namespace,
                unsigned int        metadataSize,
                MetadataEncoder     metadataEncoder,
                unsigned int        chunkSize,
                ContextType         contextType,
                CallbackHandler     callbackHandler,
                unsigned int       *contextID)
  __attribute__((warn_unused_result));
#else /* NAMESPACES */
/**
 * Open a context.
 *
 * @param [in]  session          The index session on which to open a context
 * @param [in]  metadataSize     The application metadata size to copy
 * @param [in]  metadataEncoder  A function to encode application metadata
 *                               into an index request
 * @param [in]  chunkSize        The data chunk size, for file/stream contexts
 * @param [in]  contextType      The type of context to open
 * @param [in]  callbackHandler  The function that passes results in completed
 *                               requests back to the user's callback function
 * @param [out] contextID        A point to hold the id of the new context
 *
 * @return UDS_SUCCESS or an error
 **/
int openContext(UdsIndexSession     session,
                unsigned int        metadataSize,
                MetadataEncoder     metadataEncoder,
                unsigned int        chunkSize,
                ContextType         contextType,
                CallbackHandler     callbackHandler,
                unsigned int       *contextID)
  __attribute__((warn_unused_result));
#endif /* NAMESPACES */

/**
 * Convert an internal to an external error, disabling the context if the
 * error is permanent.
 *
 * @param context   The context which had the error
 * @param errorCode The internal error code
 *
 * @return The external error code
 **/
int handleError(UdsContext *context, int errorCode)
  __attribute__((warn_unused_result));

/**
 * Convert an internal to an external error, disabling the context if the
 * error is permanent, then release the context.
 *
 * @param context   The context which had the error
 * @param errorCode The internal error code
 *
 * @return The external error code
 **/
int handleErrorAndReleaseBaseContext(UdsContext *context, int errorCode)
  __attribute__((warn_unused_result));

#if NAMESPACES
/**
 * Construct a new UdsContext, initializing fields excluding the context's
 * session.
 *
 * @param [in]  indexSession     The index session under which the context is
 *                               opened
 * @param [in]  namespace        The namespace to associate with the context
 * @param [in]  metadataSize     The application metadata size to copy
 * @param [in]  metadataEncoder  A function to encode application metadata
 *                               into an index request
 * @param [in]  chunkSize        The data chunk size, for file/stream contexts
 * @param [in]  contextType      The type of context (block, file, or stream)
 * @param [in]  callbackHandler  The function that passes results in completed
 *                               requests back to the user's callback function
 * @param [out] contextPtr       The pointer to receive the new context
 **/
int makeBaseContext(IndexSession        *indexSession,
                    const UdsNamespace  *namespace,
                    unsigned int         metadataSize,
                    MetadataEncoder      metadataEncoder,
                    unsigned int         chunkSize,
                    ContextType          contextType,
                    CallbackHandler      callbackHandler,
                    UdsContext         **contextPtr)
  __attribute__((warn_unused_result));
#else /* NAMESPACES */
/**
 * Construct a new UdsContext, initializing fields excluding the context's
 * session.
 *
 * @param [in]  indexSession     The index session under which the context is
 *                               opened
 * @param [in]  metadataSize     The application metadata size to copy
 * @param [in]  metadataEncoder  A function to encode application metadata
 *                               into an index request
 * @param [in]  chunkSize        The data chunk size, for file/stream contexts
 * @param [in]  contextType      The type of context (block, file, or stream)
 * @param [in]  callbackHandler  The function that passes results in completed
 *                               requests back to the user's callback function
 * @param [out] contextPtr       The pointer to receive the new context
 **/
int makeBaseContext(IndexSession        *indexSession,
                    unsigned int         metadataSize,
                    MetadataEncoder      metadataEncoder,
                    unsigned int         chunkSize,
                    ContextType          contextType,
                    CallbackHandler      callbackHandler,
                    UdsContext         **contextPtr)
  __attribute__((warn_unused_result));
#endif /* NAMESPACES */

/**
 * Get the non-type-specific underlying context for a given context.
 *
 * @param contextId   The id of the context making the request
 * @param contextType The type of context making the request
 * @param contextPtr  A pointer to receive the base context
 *
 * @return UDS_SUCCESS or an error code
 **/
int getBaseContext(unsigned int   contextId,
                   ContextType    contextType,
                   UdsContext   **contextPtr)
  __attribute__((warn_unused_result));

/**
 * Release the non-type-specific underlying context for a given context.
 *
 * @param context The context to release
 **/
void releaseBaseContext(UdsContext *context);

/**
 * Flush all outstanding requests on a given base context.
 *
 * @param context The context to flush
 **/
void flushBaseContext(UdsContext *context);

/**
 * Free a context.
 *
 * @param context The context to free
 **/
void freeContext(UdsContext *context);

/**
 * Flush all outstanding requests on a given context.
 *
 * @param contextId   The id of the context to flush
 * @param contextType The type of context being flushed
 *
 * @return UDS_SUCCESS or an error code
 **/
int flushContext(unsigned int contextId, ContextType contextType)
  __attribute__((warn_unused_result));

/**
 * Close a context.
 *
 * @param contextId   The id of the context to close
 * @param contextType The type of context to close
 **/
int closeContext(unsigned int contextId, ContextType contextType)
  __attribute__((warn_unused_result));

/**
 * Get the configuration associated with a given context.
 *
 * @param contextId   The id of the context
 * @param contextType The type of context to make
 * @param conf        A pointer to hold the configuration
 *
 * @return UDS_SUCCESS or an error
 **/
int getConfiguration(unsigned int      contextId,
                     ContextType       contextType,
                     UdsConfiguration *conf)
  __attribute__((warn_unused_result));

/**
 * Get the index statistics for the index associated with a given context.
 *
 * @param contextId   The id of the context
 * @param contextType The type of context to make
 * @param stats       A pointer to hold the statistics
 *
 * @return UDS_SUCCESS or an error
 **/
int getContextIndexStats(unsigned int   contextId,
                         ContextType    contextType,
                         UdsIndexStats *stats)
  __attribute__((warn_unused_result));

/**
 * Get statistics for a given context.
 *
 * @param contextId   The id of the context
 * @param contextType The type of context to make
 * @param stats       A pointer to hold the statistics
 *
 * @return UDS_SUCCESS or an error
 **/
int getContextStats(unsigned int     contextId,
                    ContextType      contextType,
                    UdsContextStats *stats)
  __attribute__((warn_unused_result));

/**
 * Reset the statistics for a given context.
 *
 * @param contextId   The id of the context
 * @param contextType The type of context to make
 *
 * @return UDS_SUCCESS or an error
 **/
int resetStats(unsigned int contextId, ContextType contextType)
  __attribute__((warn_unused_result));

/**
 * Change the maximum number of outstanding requests.
 *
 * @param contextId    The id of the context making the request
 * @param contextType  The type of context
 * @param maxRequests  The new maximum number of pending requests
 *
 * @return              Either UDS_SUCCESS or an error code.
 *
 **/
int setRequestQueueLimit(unsigned int contextId,
                         ContextType  contextType,
                         unsigned int maxRequests)
  __attribute__((warn_unused_result));

/**
 * Change the function/algorithm used for hashing chunks of data.
 *
 * @param contextId    The id of the context making the request
 * @param contextType  The type of context
 * @param algorithm    The hash algorthim selection
 *
 * @return             Either UDS_SUCCESS or an error code.
 *
 **/
int setChunkNameAlgorithm(unsigned int     contextId,
                          ContextType      contextType,
                          UdsHashAlgorithm algorithm)
  __attribute__((warn_unused_result));

/**
 * Calculates the name (hash) of a chunk of data by using
 * the hashing function that was selected for a context.
 *
 * @param contextId    The id of the context making the request
 * @param contextType  The type of context
 * @param data         A pointer to the opaque data
 * @param size         The size of the data, in bytes
 *
 * @return             The calculated chunk name
 *
 **/
UdsChunkName generateChunkName(unsigned int     contextId,
                               ContextType      contextType,
                               const void      *data,
                               size_t           size)
  __attribute__((warn_unused_result));

/**
 * Register a callback for deduplication advice on a context.
 *
 * @param contextID        The id of the context
 * @param contextType      The type of the context
 * @param callbackFunction The callback function (set to NULL to clear
 *                         the current callback)
 * @param callbackArgument Opaque, client supplied data which will be
 *                         passed to the callback function for each
 *                         request
 *
 * @return UDS_SUCCESS or an error code
 **/
int registerDedupeCallback(unsigned int           contextID,
                           ContextType            contextType,
                           IndexCallbackFunction *callbackFunction,
                           void                  *callbackArgument)
  __attribute__((warn_unused_result));

/**
 * Dispatch a control request to a client context on the callback thread.
 *
 * @param request  the control request to dispatch
 *
 * @return UDS_SUCCESS or an error code
 **/
int dispatchContextControlRequest(Request *request)
  __attribute__((warn_unused_result));

#endif /* CONTEXT_H */
