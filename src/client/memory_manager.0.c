/*! ***************************************************************************
 * \file    memory_manager.0.c
 * \brief
 * 
 * \date    December 24, 2011
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
initMmgrClient()
{    
    int mmgrShmFd   = atoi(environ[2]);
    int mmgrShmSize = atoi(environ[3]);

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
	    if (initMmgrClient() == -1) return -1;	\
	}						\
    } while (0)

int
gvmmgrAlloc(size_t *offset, size_t length)
{
    GVSERcallid callId;
    int         status;

    initIfNotDoneAlready();
    gvdisMakeCurrent(dispatcher);

    callId = gvserCall(GVSER_MMGR_ALLOC);
    gvserInParameter(&length, sizeof(size_t));
    gvserEndCall();

    gvserReturn(callId);
    gvserReturnValue(&status, sizeof(int));
    gvserOutParameter(offset, sizeof(size_t));
    gvserEndReturn();

    return status;
}

int
gvmmgrFree(size_t offset)
{
    GVSERcallid callId;
    int         status;

    initIfNotDoneAlready();
    gvdisMakeCurrent(dispatcher);

    callId = gvserCall(GVSER_MMGR_FREE);
    gvserInParameter(&offset, sizeof(size_t));
    gvserEndCall();

    gvserReturn(callId);
    gvserReturnValue(&status, sizeof(int));
    gvserEndReturn();

    return 0;
}
