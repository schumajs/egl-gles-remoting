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
#include "serializer.h"
#include "server_dispatcher.h"
#include "server_janitor.h"
#include "server_state_tracker.0.h"
#include "transport.h"

extern GVdispatchfunc eglGlesJumpTable[1];

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
	if (gvCall(transport, NULL, NULL, &callId) == -1)
	{
	    THROW(e0, "gvCall");
	}

	if (gvGetData(transport, &offset, sizeof(size_t)) == -1)
	{
	    THROW(e0, "gvGetData");
	}

	if (gvGetData(transport, &length, sizeof(size_t)) == -1)
	{
	    THROW(e0, "gvGetData");
	}

	if (gvEndCall(transport, NULL) == -1) {
	    THROW(e0, "gvEndCall");
	}

	status = gvBonjour(offset, length);

	if (gvReturn(transport, NULL, &callId) == -1)
	{
	    THROW(e0, "gvReturn");
	}

	if (gvPutData(transport, &status, sizeof(int)) == -1)
	{
	    THROW(e0, "gvPutData");
	}

	if (gvEndReturn(transport, NULL) == -1)
	{
	    THROW(e0, "gvReturn");
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
	if (gvCall(transport, NULL, NULL, &callId) == -1)
	{
	    THROW(e0, "gvCall");
	}

	if (gvGetData(transport, &offset, sizeof(size_t)) == -1)
	{
	    THROW(e0, "gvGetData");
	}

	if (gvEndCall(transport, NULL) == -1) {
	    THROW(e0, "gvEndCall");
	}

	status = gvAuRevoir(offset);

	if (gvReturn(transport, NULL, &callId) == -1)
	{
	    THROW(e0, "gvReturn");
	}

	if (gvPutData(transport, &status, sizeof(int)) == -1)
	{
	    THROW(e0, "gvPutData");
	}

	if (gvEndReturn(transport, NULL) == -1)
	{
	    THROW(e0, "gvReturn");
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

int
gvStartJanitor(GVjanitorptr *newJanitor, GVshmptr vmShm)
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

	if (gvCreateShm(&(janitor->public.janitorShm), 3 * 4096) == -1)
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
	    if (gvCreateTransport(&(janitor->janitorTransport),
				  janitor->public.janitorShm,
				  0,
				  janitor->public.janitorShm->size) == -1)
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

	    return 0;
	}
	else if (janitor->janitorPid > 0)
	{
	    /*
	     * Dashboard process
	     */

	    *newJanitor = (GVjanitorptr) janitor;
	
	    return 1;       
	}
	else if (janitor->janitorPid < 0)
	{
	    THROW(e0, "fork");
	}
    }
    CATCH (e0)
    {
	return -1;
    }
    CATCH (e1)
    {
	exit(2);
    }

    return -1;
}

int
gvBonjour(size_t offset, size_t length)
{
    GVtransportptr  transport;
 
    TRY
    {
	/* Create transport */
	if (gvCreateTransport(&transport,
			      janitor->public.vmShm,
			      offset,
			      length) == -1)
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
    GVjanitorstateptr janitorState;

    TRY
    {
	if (gvGetJanitorState(offset, &janitorState) == -1)
	{
	    THROW(e0, "gvGetJanitorState");
	}

	if (pthread_cancel(janitorState->thread) != 0)
	{
	    THROW(e0, "pthread_cancel");
	}

	if (gvDestroyTransport(janitorState->transport) == -1)
	{
	    THROW(e0, "gvDestroyTransport");
	}

	if (gvDelJanitorState(offset) == -1)
	{
	    THROW(e0, "gvDelJanitorState");
	}
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
