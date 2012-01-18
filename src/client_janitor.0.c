/*! ***************************************************************************
 * \file    client_janitor.0.c
 * \brief
 * 
 * \date    January 6, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <stdlib.h>

#include "serializer.h"
#include "error.h"
#include "janitor.h"
#include "transport.h"

extern char           **environ;

static int              processInitiated = 0;
static GVtransportptr   transport;
static GVlockptr        callLock;
static GVlockptr        returnLock;

static int
initJanitorClient()
{    
    GVshmptr janitorShm;
    int      janitorShmFd   = atoi(environ[4]);
    int      janitorShmSize = atoi(environ[5]);
    
    TRY
    {
	if ((janitorShm = malloc(sizeof(GVshmptr))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	janitorShm->id   = janitorShmFd;
	janitorShm->size = janitorShmSize;

	if (gvCreateTransport(&transport, janitorShm, 0, janitorShmSize) == -1)
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
	    if (initJanitorClient() == -1) return -1;	\
	    processInitiated = 1;			\
	}						\
    } while (0)

int
gvBonjour(size_t offset, size_t length)
{
    GVcmdid   cmdId = GV_CMDID_JANITOR_BONJOUR;
    GVcallid  callId;
    int       status;

    initIfNotDoneAlready();

    gvCall(transport, callLock, &cmdId, &callId);
    gvPutData(transport, &offset, sizeof(size_t));
    gvPutData(transport, &length, sizeof(size_t));
    gvEndCall(transport, callLock);

    gvReturn(transport, returnLock, &callId);
    gvGetData(transport, &status, sizeof(int));
    gvEndReturn(transport, returnLock);

    return status;
}

int
gvAuRevoir(size_t offset)
{
    GVcmdid  cmdId = GV_CMDID_JANITOR_AUREVOIR;
    GVcallid callId;
    int      status;

    initIfNotDoneAlready();

    gvCall(transport, callLock, &cmdId, &callId);
    gvPutData(transport, &offset, sizeof(size_t));
    gvEndCall(transport, callLock);

    gvReturn(transport, returnLock, &callId);
    gvGetData(transport, &status, sizeof(int));
    gvEndReturn(transport, returnLock);

    return status;
}
