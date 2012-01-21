/*! ***************************************************************************
 * \file    server_janitor.h
 * \brief   
 * 
 * \date    January 6, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <signal.h>
#include <stdlib.h>

#include "error.h"
#include "process_state_map.h"
#include "server_dispatcher.h"
#include "server_janitor.h"
#include "server_serializer.h"
#include "server_state_tracker.0.h"
#include "transport.h"

extern GVdispatchfunc eglGlesJumpTable[11];

/* ****************************************************************************
 * Janitor
 */

struct Janitor {
    struct GVjanitor public;
    pid_t            janitorPid;
    GVtransportptr   janitorTransport;
};

#define castJanitor(janitor) \
    ((struct Janitor *)janitor)

static struct Janitor        *janitor;

/* ****************************************************************************
 * Dispatching functions
 */

static
void _gvBonjour()
{
    GVtransportptr transport = janitor->janitorTransport;

    GVcallid       callId;
    int            status;
    size_t         offset;
    size_t         length;

    TRY
    {
	if (gvReceiveData(transport, &callId, sizeof(GVcallid)) == -1)
	{
	    THROW(e0, "gvReceiveData");
	}

	if (gvReceiveData(transport, &offset, sizeof(size_t)) == -1)
	{
	    THROW(e0, "gvReceiveData");
	}

	if (gvReceiveData(transport, &length, sizeof(size_t)) == -1)
	{
	    THROW(e0, "gvReceiveData");
	}

	status = gvBonjour(offset, length);

	if (gvStartSending(transport, NULL, callId) == -1)
	{
	    THROW(e0, "gvStartSending");
	}

	if (gvSendData(transport, &status, sizeof(int)) == -1)
	{
	    THROW(e0, "gvSendData");
	}
    }
    CATCH (e0)
    {
	return;
    }
}

static
void _gvAuRevoir()
{
    GVtransportptr transport = janitor->janitorTransport;

    GVcallid       callId;
    int            status;
    size_t         offset;

    TRY
    {
	if (gvReceiveData(transport, &callId, sizeof(GVcallid)) == -1)
	{
	    THROW(e0, "gvReceiveData");
	}

	if (gvReceiveData(transport, &offset, sizeof(size_t)) == -1)
	{
	    THROW(e0, "gvReceiveData");
	}

	status = gvAuRevoir(offset);

	if (gvStartSending(transport, NULL, callId) == -1)
	{
	    THROW(e0, "gvReturn");
	}

	if (gvSendData(transport, &status, sizeof(int)) == -1)
	{
	    THROW(e0, "gvSendData");
	}
    }
    CATCH (e0)
    {
	return;
    }
}

/* ****************************************************************************
 * Jump table
 */

static GVdispatchfunc gvJanitorJumpTable[2] = {
    _gvBonjour,
    _gvAuRevoir
};

/* ****************************************************************************
 * Server janitor implementation
 */

static void
sigtermHandler()
{
    TRY
    {
	/* Destroy server transport (+ detach shared memory) */
	if (gvDestroyTransport(janitor->janitorTransport) == -1)
	{
	    THROW(e0, "gvDestroyTransport");
	}

	/* Destroy server shared memory */
	if (gvDestroyShm(janitor->public.janitorShm) == -1)
	{
	    THROW(e0, "gvDestroyShm");
	}

	free(janitor);
    }
    CATCH (e0)
    {
	exit(2);
    }

    exit(3);
}

GVjanitorptr
gvStartJanitor(GVshmptr vmShm)
{
    TRY
    {
	if ((janitor = malloc(sizeof(struct Janitor))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	janitor->public.vmShm = vmShm;

	/* Create shared memory in parent process, so corresponding file
	 * descriptor will be available to child processes.
	 */

	if ((janitor->public.janitorShm = gvCreateShm(3 * 4096)) == NULL)
	{
	    THROW(e0, "gvCreateShm");
	}

	if (!(janitor->janitorPid = fork()))
	{
	    /*
	     * Server process
	     */

	    /* Register "shutdown hook" */
	    if (signal(SIGTERM, sigtermHandler) == SIG_ERR)
	    {
		THROW(e1, "signal");
	    }

	    /* Create transport */
	    if ((janitor->janitorTransport
		 = gvCreateTransport(janitor->public.janitorShm,
				     0,
				     janitor->public.janitorShm->size)) == NULL)
	    {
		THROW(e1, "gvCreateTransport");
	    }

	    /* Start dispatching loop */	
	    if (gvDispatchLoop(janitor->janitorTransport,
			       gvJanitorJumpTable,
			       1) == -1)
	    {
		THROW(e1, "gvDispatchLoop");
	    }

	    return NULL;
	}
	else if (janitor->janitorPid > 0)
	{
	    /*
	     * Dashboard process
	     */

	    return (GVjanitorptr) janitor;
	}
	else if (janitor->janitorPid < 0)
	{
	    THROW(e0, "fork");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }
    CATCH (e1)
    {
	exit(2);
    }

    return NULL;
}

int
gvBonjour(size_t offset,
	  size_t length)
{
    GVtransportptr  transport;
 
    TRY
    {
	/* Create transport */
	if ((transport
	     = gvCreateTransport(janitor->public.vmShm,
				 offset,
				 length)) == NULL)
	{
	    THROW(e0, "gvCreateTransport");
	}

	/* Start dispatching loop */	
	if (gvDispatchLoop(transport, eglGlesJumpTable, 0) == -1)
	{
	    THROW(e0, "gvDispatchLoop");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvAuRevoir(size_t offset)
{
    GVoffsetstateptr offsetState;

    TRY
    {
	if ((offsetState = gvGetOffsetState(offset)) == NULL)
	{
	    THROW(e0, "gvGetProcessState");
	}

	if (gvDelOffsetState(offset) == -1)
	{
	    THROW(e0, "gvDelJanitorState");
	}

	if (pthread_cancel(offsetState->thread) != 0)
	{
	    THROW(e0, "pthread_cancel");
	}

	if (gvDestroyTransport(offsetState->transport) == -1)
	{
	    THROW(e0, "gvDestroyTransport");
	}

	/* free(processState); */
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvStopJanitor(GVjanitorptr janitor)
{
    TRY
    {
	if (kill(castJanitor(janitor)->janitorPid, SIGTERM) == -1)
	{
	    THROW(e0, "kill");
	}

	free(janitor);
    }
    CATCH (e0)
    {
	return 1;
    }

    return 0;
}
