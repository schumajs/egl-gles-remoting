/*! ***************************************************************************
 * \file    server.h
 * \brief   
 * 
 * \date    January 6, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "transport.h"

#include "server/dispatcher.h"
#include "server/serializer.h"
#include "server/server.h"

extern GVDISfunc eglGlesJumpTable[1];

/* ****************************************************************************
 * Context
 */

struct Context {
    struct GVSRVcontext public;
    GVSHMshmptr         vmShm;
    pid_t               srvPid;
    GVTRPtransportptr   srvTransport;
    GVDISdispatcherptr  srvDispatcher;
};

static struct Context *context;

/* ****************************************************************************
 * Dispatching functions
 */

static
void _gvsrvHandshake()
{
    GVSERcallid callId;
    int         status;
    size_t      offset; 
    size_t      length;

    callId = gvserCall();
    gvserInData(&offset, sizeof(size_t));
    gvserInData(&length, sizeof(size_t));
    gvserEndCall();

    status = gvsrvHandshake(offset, length);

    gvserReturn(callId);
    gvserOutData(&status, sizeof(int));
    gvserEndReturn();
}

/* ****************************************************************************
 * Jump table
 */

static GVDISfunc gvsrvJumpTable[1] = {
    _gvsrvHandshake
};

/* ****************************************************************************
 * Server server implementation
 */

static void
sigtermHandler()
{
    /* Destroy server transport (+ detach shared memory) */
    if (gvtrpDestroy(context->srvTransport) == -1)
    {
	perror("gvtrpDestroy");
	exit(2);
    }

    /* Destroy server shared memory */
    if (gvshmDestroy(context->public.srvShm) == -1)
    {
	perror("gvshmDestroy");
	exit(2);
    }

    free(context);

    exit(3);
}

int
gvsrvCreate(GVSRVcontextptr *newServer, GVSHMshmptr vmShm)
{
    if ((context = malloc(sizeof(struct Context))) == NULL)
    {
	perror("malloc");
	return -1;
    }

    context->vmShm = vmShm;

    /*
     * Create shared memory in parent process, so corresponding file
     * descriptor will be available to child processes.
     */

    /* Min. transport size ... */
    context->public.srvShmSize = 3 * 4096;

    if (gvshmCreate(&(context->public.srvShm),
		    context->public.srvShmSize)
	== -1)
    {
	perror("gvshmCreate");
	return -1;
    }

    if (!(context->srvPid = fork()))
    {
	/*
         * Server process
         */

	if (signal(SIGTERM, sigtermHandler) == SIG_ERR)
	{
	    perror("signal");
	    exit(2);
	}

	/* Create transport */
	if (gvtrpCreate(&(context->srvTransport),
			context->public.srvShm,
			0,
			context->public.srvShmSize)
	    == -1)
	{
	    perror("gvtrpCreate");
	    exit(2);
	}

	/* Create dispatcher */
	if (gvdisCreate(&(context->srvDispatcher),
			context->srvTransport) == -1)
	{
	    perror("gvdisCreate");
	    exit(2);
	}
	
	/* Start dispatching loop */	
	if (gvdisMakeCurrent(context->srvDispatcher) == -1)
	{
	    perror("gvdisMakeCurrent");
	    exit(2);
	}

	if (gvdisDispatchLoop(NULL, gvsrvJumpTable, 1) == -1)
	{
	    perror("gvdisDispatchLoop");
	    exit(2);
	}

	return 0;
    }
    else if (context->srvPid > 0)
    {
	/*
	 * Dashboard process
         */

	*newServer = (GVSRVcontextptr) context;
	
	return 1;
    }
    else if (context->srvPid < 0)
    {
	perror("fork");
    }

    return -1;
}

int
gvsrvHandshake(size_t offset, size_t length)
{
    GVTRPtransportptr  transport;
    GVDISdispatcherptr dispatcher;

    if (gvtrpCreate(&transport,
		    context->vmShm, offset, length) == -1)
    {
	perror("gvtrpCreate");
	return -1;
    }

    if ((gvdisCreate(&dispatcher, transport) == -1))
    {
	perror("gvdisCreate");
	exit(2);
    }

    if (gvdisDispatchLoop(dispatcher, eglGlesJumpTable, 0) == -1)
    {
	perror("gvdisDispatchLoop");
	exit(2);
    }

    return 0;
}

int
gvsrvDestroy(GVSRVcontextptr server)
{
    if (kill(((struct Context *)context)->srvPid, SIGTERM) == -1)
    {
	perror("kill");
	return -1;
    }

    free(server);

    return 0;
}
