/*! ***************************************************************************
 * \file    client_heap_manager.0.c
 * \brief
 * 
 * \date    December 24, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <stdlib.h>

#include "client_serializer.h"
#include "error.h"
#include "heap_manager.h"
#include "lock.h"
#include "shared_memory.h"
#include "shm_stream_transport.h"

static int            processInitiated = 0;

static GVtransportptr transport;
static GVlockptr      ocLock;
static GVlockptr      icLock;

static int
initHeapMgrClient()
{    
    GVshmptr heapMgrShm;
    int      heapMgrShmFd   = atoi(getenv("GV_HMGR_SHM_FD"));
    int      heapMgrShmSize = atoi(getenv("GV_HMGR_SHM_SIZE"));
    
    TRY
    {
	if ((heapMgrShm = malloc(sizeof(struct GVshm))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	heapMgrShm->id   = heapMgrShmFd;
	heapMgrShm->size = heapMgrShmSize;
	
	if ((transport
	     = gvCreateShmStreamTransport(heapMgrShm,
					  0,
					  heapMgrShmSize)) == NULL)
	{
	    THROW(e0, "gvCreateTransport");
	}

	ocLock = transport->callChanel->exclusiveAccess;
	icLock = transport->returnChanel->exclusiveAccess;
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

#define initIfNotDoneAlready()				\
    do {						\
	if (!processInitiated)				\
	{						\
	    if (initHeapMgrClient() == -1) return -1;	\
	    processInitiated = 1;			\
	}						\
    } while (0)

size_t
gvAlloc(size_t length)
{
    GVcallid callId;
    size_t   offset;

    initIfNotDoneAlready();

    callId = gvStartSending(transport, ocLock, GV_CMDID_HMGR_ALLOC);
    gvSendData(transport, &length, sizeof(size_t));
    gvStopSending(transport, ocLock);

    gvStartReceiving(transport, icLock, callId);
    gvReceiveData(transport, &offset, sizeof(size_t));
    gvStopReceiving(transport, icLock);

    return offset;
}

int
gvFree(size_t offset)
{
    GVcallid callId;
    int      status;

    initIfNotDoneAlready();

    callId = gvStartSending(transport, ocLock, GV_CMDID_HMGR_FREE);
    gvSendData(transport, &offset, sizeof(size_t));
    gvStopSending(transport, ocLock);

    gvStartReceiving(transport, icLock, callId);
    gvReceiveData(transport, &status, sizeof(int));
    gvStopReceiving(transport, icLock);

    return status;
}
