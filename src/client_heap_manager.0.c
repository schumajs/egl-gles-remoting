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

#include "error.h"
#include "heap_manager.h"
#include "lock.h"
#include "serializer.h"
#include "transport.h"

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
	if ((heapMgrShm = malloc(sizeof(GVshmptr))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	heapMgrShm->id   = heapMgrShmFd;
	heapMgrShm->size = heapMgrShmSize;

	if (gvCreateTransport(&transport, heapMgrShm, 0, heapMgrShmSize) == -1)
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

int
gvAlloc(size_t *offset, size_t length)
{
    GVcallid cmdId = GV_CMDID_HMGR_ALLOC;
    GVcallid callId;
    int      status;

    initIfNotDoneAlready();

    gvCall(transport, callLock, &cmdId, &callId);
    gvPutData(transport, &length, sizeof(size_t));
    gvEndCall(transport, callLock);

    gvReturn(transport, returnLock, &callId);
    gvGetData(transport, &status, sizeof(int));
    gvGetData(transport, offset, sizeof(size_t));
    gvEndReturn(transport, returnLock);

    puts("CLIENT ALLOC C");

    return status;
}

int
gvFree(size_t offset)
{
    GVcallid cmdId = GV_CMDID_HMGR_FREE;
    GVcallid callId;
    int      status;

    initIfNotDoneAlready();

    gvCall(transport, callLock, &cmdId, &callId);
    gvPutData(transport, &offset, sizeof(size_t));
    gvEndCall(transport, callLock);

    printf("%i %i\n", cmdId, callId);

    gvReturn(transport, returnLock, &callId);
    gvGetData(transport, &status, sizeof(int));
    gvEndReturn(transport, returnLock);

    return status;
}
