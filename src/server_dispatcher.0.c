/*! ***************************************************************************
 * \file    server_dispatcher.0.c
 * \brief
 * 
 * \date    January 9, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#define _MULTI_THREADED
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "sleep.h"
#include "serializer.h"
#include "server_dispatcher.h"
#include "server_state_tracker.h"

/* ***************************************************************************
 * State tracking
 */

typedef GVtransportptr ThreadState;

/* ***************************************************************************
 * pthread - thread specific data
 */

pthread_mutex_t      initTerminateLock = PTHREAD_MUTEX_INITIALIZER;
static int           threadCounter     =  0;
static pthread_key_t threadSpecificKey = -1;

static void
threadSpecificDataDestructor(void *threadSpecificData) {
    pthread_setspecific(threadSpecificKey, NULL);
}

/* ***************************************************************************
 * Server state tracker implementation
 */

#define getCurrentThreadState() \
    ((ThreadState) pthread_getspecific(threadSpecificKey))

#define setCurrentThreadState(state) \
    pthread_setspecific(threadSpecificKey, state)

int
gvInitStateTracker()
{
    TRY ()
    {
	if (pthread_mutex_lock(&initTerminateLock) != 0)
	{
	    THROW(e0, "pthread_mutex_lock");
	}

	if (threadSpecificKey == -1)
	{
	    if (pthread_key_create(&threadSpecificKey,
				   threadSpecificDataDestructor) != 0)
	    {
		THROW(e1, "pthread_key_create");
	    }
	}
    
	threadCounter++;

	if (pthread_mutex_unlock(&initTerminateLock) != 0)
	{
	    THROW(e1, "pthread_mutex_unlock");
	}
    }
    CATCH (e0)
    {
	return -1;
    }
    CATCH (e1)
    {
	/* Try to unlock. At this point we can't do anything if unlocking
         * fails, so just ignore errors.
         */
	pthread_mutex_unlock(&initTerminateLock);
	return -1;
    }
   
    return 0;
}

int
gvGetCurrent(GVtransportptr *transport)
{
    ThreadState state; 

    TRY ()
    {
	if ((state = getCurrentThreadState()) == NULL)
	{
	    THROW(e0, "no current context");
	}

	*transport = state;
    }
    CATCH(e0)
    {
	return -1;
    }

    return 0;
}

int
gvSetCurrent(GVtransportptr transport)
{
    ThreadState state;

    TRY ()
    {
	if ((state = getCurrentThreadState()) != NULL)
	{
	    if (setCurrentThreadState(NULL) != -1)
	    {
		THROW(e0, "pthread_setspecific");
	    }
	}

	state = transport;

	if (setCurrentThreadState(state) != 0)
	{
	    THROW(e0, "pthread_setspecific");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvTerminateStateTracker()
{
    TRY ()
    {
	if (pthread_mutex_lock(&initTerminateLock) != 0)
	{
	    THROW(e0, "pthread_mutex_lock");
	}

	if (threadCounter == 1)
	{
	    if (pthread_key_delete(threadSpecificKey) != 0)
	    {
		THROW(e1, "pthread_key_delete");
	    }
	
	    threadSpecificKey = -1;
	}

	threadCounter--;

	if (pthread_mutex_unlock(&initTerminateLock) != 0)
	{
	    THROW(e1, "pthread_mutex_unlock");
	}
    }
    CATCH (e0)
    {
	return -1;
    }
    CATCH (e1)
    {
	/* Try to unlock. At this point we can't do anything if unlocking
         * fails, so just ignore errors.
         */
	pthread_mutex_unlock(&initTerminateLock);
	return -1;
    } 

    return 0;
}

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
    struct ThreadArg *threadArg;

    TRY ()
    {
	threadArg = (struct ThreadArg *)data;

	if (gvSetCurrent(threadArg->transport) == -1)
	{
	    THROW(e0, "gvSetCurrent");
	}

	while (1)
	{
	    if (gvRead(threadArg->transport->callBuffer,
		       &cmdId, sizeof(GVcmdid)) == -1)
	    {
		THROW(e0, "gvRead");
	    }

	    threadArg->jumpTable[cmdId]();

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

    TRY ()
    {
	if ((threadArg = malloc(sizeof(struct ThreadArg))) == NULL)
	{
	    THROW(e0, "malloc");
	} 

	if (transport == NULL)
	{
	    /* Inherit transport of parent thread */
	    if (gvGetCurrent(&transport) == -1)
	    {
		THROW(e0, "gvGetCurrent");
	    }
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
