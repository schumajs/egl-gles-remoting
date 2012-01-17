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
#include "serializer.h"
#include "server_dispatcher.h"
#include "server_state_tracker.0.h"
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
    GVcmdid             cmdId;
    struct ThreadArg   *threadArg;
    GVtransportptr      transport;
    GVdispatchfunc     *jumpTable;

    TRY
    {
	threadArg = (struct ThreadArg *)data;

	transport = threadArg->transport;
	jumpTable = threadArg->jumpTable;
	
	free(threadArg);

	if (gvInitThreadStateMap() == -1)
	{
	    THROW(e0, "gvInitThreadState");
	}

	gvSetCurrentThreadTransport(transport);

	gvPutJanitorState(transport->offset, pthread_self(), transport);

	printf("PID %i OFFSET %zu THREAD %zu\n",
	       getpid(), transport->offset, pthread_self());

	while (1)
	{
	    if (gvRead(transport->callBuffer, &cmdId, sizeof(GVcmdid)) == -1)
	    {
		THROW(e0, "gvRead");
	    }

	    jumpTable[cmdId]();

	    gvSleep(0, 1000);
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return NULL;
}

int
gvDispatchLoop(GVtransportptr transport,
	       GVdispatchfunc jumpTable[],
	       int            joinThread)
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

	if (joinThread)
	{
	    if (pthread_join(thread, NULL) != 0)
	    {
		THROW(e0, "pthread_join");
	    }
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}
