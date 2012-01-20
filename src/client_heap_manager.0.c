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
#include "transport.h"
#include "shared_memory.h"

extern char           **environ;

static int              processInitiated = 0;

static GVtransportptr   transport;
static GVlockptr        callLock;
static GVlockptr        returnLock;

static int
initHeapMgrClient()
{    
    GVshmptr heapMgrShm;
    int      heapMgrShmFd   = atoi(environ[2]);
    int      heapMgrShmSize = atoi(environ[3]);
    
    TRY
    {
	if ((heapMgrShm = malloc(sizeof(struct GVshm))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	heapMgrShm->id   = heapMgrShmFd;
	heapMgrShm->size = heapMgrShmSize;

	if ((transport = gvCreateTransport(heapMgrShm,
					   0,
					   heapMgrShmSize)) == NULL)
	{
	    THROW(e0, "gvCreateTransport");
	}

	callLock = transport->callBuffer->clientLock;
	returnLock = transport->returnBuffer->clientLock;
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

    callId = gvStartSending(transport, callLock, GV_CMDID_HMGR_ALLOC);
    gvSendData(transport, &length, sizeof(size_t));
    gvStopSending(transport, callLock);

    gvStartReceiving(transport, returnLock, callId);
    gvReceiveData(transport, &offset, sizeof(size_t));
    gvStopReceiving(transport, returnLock);

    return offset;
}

int
gvFree(size_t offset)
{
    GVcallid callId;
    int      status;

    initIfNotDoneAlready();

    callId = gvStartSending(transport, callLock, GV_CMDID_HMGR_FREE);
    gvSendData(transport, &offset, sizeof(size_t));
    gvStopSending(transport, callLock);

    gvStartReceiving(transport, returnLock, callId);
    gvReceiveData(transport, &status, sizeof(int));
    gvStopReceiving(transport, returnLock);

    return status;
}
