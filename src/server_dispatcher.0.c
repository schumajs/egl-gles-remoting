/*! ***************************************************************************
 * \file    server_dispatcher.0.c
 * \brief
 * 
 * \date    January 9, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#define _MULTI_THREADED
#include <pthread.h>
#include <stdlib.h>

#include "error.h"
#include "process_state_map.h"
#include "sleep.h"
#include "server_dispatcher.h"
#include "server_serializer.h"
#include "server_state_tracker.h"
#include "thread_state_map.h"

/* ***************************************************************************
 * Server dispatcher implementation
 */

struct ThreadArg {
    GVtransportptr  transport;
    GVdispatchfunc *jumpTable;
};

static void
*dispatchLoopThread(void *data)
{
    GVcmdid           cmdId;
    GVdispatchfunc   *jumpTable;
    struct ThreadArg *threadArg;
    GVtransportptr    transport;

    TRY
    {
	if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) != 0)
	{
	    THROW(e0, "pthread_setcancelstate");
	}

	threadArg = (struct ThreadArg *)data;

	transport = threadArg->transport;
	jumpTable = threadArg->jumpTable;
	
	free(threadArg);

	if (gvSetCurrentThreadTransport(transport) == -1)
	{
	    THROW(e0, "gvSetThreadTransport");
	}

	while (1)
	{
	    // printf("PID %i THREAD %ul IS LISTENING\n", getpid(), pthread_self());
	    
	    if ((cmdId = gvStartReceiving(transport, NULL)) == -1)
	    {
		THROW(e0, "gvStartReceiving");
	    }

	    // printf("PID %i THREAD %ul GOT %i\n", getpid(), pthread_self(), cmdId);

	    jumpTable[cmdId]();
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return NULL;
}

unsigned long
gvDispatchLoop(GVtransportptr transport,
	       GVdispatchfunc jumpTable[])
{
    pthread_t         thread;
    struct ThreadArg *threadArg;

    TRY
    {
	if ((threadArg = malloc(sizeof(struct ThreadArg))) == NULL)
	{
	    THROW(e0, "malloc");
	} 

	threadArg->transport = transport;
	threadArg->jumpTable = jumpTable;

	if (pthread_create(&thread, NULL, dispatchLoopThread, threadArg) != 0)
	{
	    THROW(e0, "pthread_create");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return thread;
}
