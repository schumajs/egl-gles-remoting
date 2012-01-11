/*! ***************************************************************************
 * \file    server.0.c
 * \brief
 * 
 * \date    January 6, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <stdio.h>

#include "dispatcher.h"
#include "memory_manager.h"
#include "transport.h"

#include "client/serializer.h"

extern char **environ;

static GVDISdispatcherptr dispatcher;
static int                initiated = 0;
static GVTRPtransportptr  transport;

static int
initSrvClient()
{    
    int mmgrShmFd   = atoi(environ[4]);
    int mmgrShmSize = atoi(environ[5]);

    if (gvtrpCreate(&transport, &mmgrShmFd, 0, mmgrShmSize) == -1)
    {
	perror("gvtrpCreate");
	return -1;
    }

    if (gvdisCreate(&dispatcher, transport) == -1)
    {
	perror("gvdisCreate");
	return -1;
    }

    initiated = 1;

    return 0;
}

#define initIfNotDoneAlready()				\
    do {						\
	if (!initiated)					\
	{						\
	    if (initSrvClient() == -1) return -1;	\
	}						\
    } while (0)

int
gvsrvHandshake(size_t offset, size_t length)
{
    GVSERcallid callId;
    int         status;

    initIfNotDoneAlready();
    gvdisMakeCurrent(dispatcher);

    callId = gvserCall(GVSER_SRV_HANDSHAKE);
    gvserInData(&offset, sizeof(size_t));
    gvserInData(&length, sizeof(size_t));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(int));
    gvserEndReturn();

    return status;
}
