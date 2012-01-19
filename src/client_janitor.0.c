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

#include "client_serializer.h"
#include "error.h"
#include "janitor.h"
#include "lock.h"
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

	if ((transport = gvCreateTransport(janitorShm,
					   0,
					   janitorShmSize)) == NULL)
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
    GVcallid  callId;
    int       status;

    initIfNotDoneAlready();

    callId = gvStartSending(transport, callLock, GV_CMDID_JANITOR_BONJOUR);
    gvSendData(transport, &offset, sizeof(size_t));
    gvSendData(transport, &length, sizeof(size_t));
    gvStopSending(transport, callLock);

    gvStartReceiving(transport, returnLock, callId);
    gvReceiveData(transport, &status, sizeof(int));
    gvStopReceiving(transport, returnLock);

    return status;
}

int
gvAuRevoir(size_t offset)
{
    GVcallid callId;
    int      status;

    initIfNotDoneAlready();

    callId = gvStartSending(transport, callLock, GV_CMDID_JANITOR_AUREVOIR);
    gvSendData(transport, &offset, sizeof(size_t));
    gvStopSending(transport, callLock);

    gvStartReceiving(transport, returnLock, callId);
    gvReceiveData(transport, &status, sizeof(int));
    gvStopReceiving(transport, returnLock);

    return status;
}
