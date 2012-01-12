/*! ***************************************************************************
 * \file    dispatcher.0.c
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

#include "dispatcher.h"

/* ***************************************************************************
 * pthread specific stuff
 */

/* Used to synchronize access to gvdisCreate and gvdisDestroy */
pthread_mutex_t      createAndDestroyLock = PTHREAD_MUTEX_INITIALIZER;

static pthread_key_t threadSpecificKey = -1;
static int           refCounter        =  0;

static void
threadSpecificDataDestructor(void *threadSpecificData) {
    pthread_setspecific(threadSpecificKey, NULL);
}

/* ***************************************************************************
 * Dispatcher implementation
 */

int
gvdisCreate(GVDISdispatcherptr *newContext, GVTRPtransportptr transport)
{
    pthread_mutex_lock(&createAndDestroyLock);
    
    if (threadSpecificKey == -1)
    {
	if (pthread_key_create(&threadSpecificKey,
			       threadSpecificDataDestructor) != 0)
	{
	    perror("pthread_key_create");
	    goto error;
	}
    }

    if ((*newContext = malloc(sizeof(struct GVDISdispatcher))) == NULL)
    {
	perror("malloc");
	goto error;
    }

    (*newContext)->transport = transport;

    refCounter++;

/* success: */
    pthread_mutex_unlock(&createAndDestroyLock);
    return 0;

error:
    pthread_mutex_unlock(&createAndDestroyLock);
    return -1;
}

GVDISdispatcherptr
gvdisGetCurrent()
{
    return (GVDISdispatcherptr) pthread_getspecific(threadSpecificKey);
}

int
gvdisMakeCurrent(GVDISdispatcherptr context)
{
    if (pthread_setspecific(threadSpecificKey, context) != 0)
    {
	perror("pthread_setspecific");
	return -1;
    }

    return 0;
}

int
gvdisDestroy(GVDISdispatcherptr dispatcher)
{
    pthread_mutex_lock(&createAndDestroyLock);

    if (refCounter == 1)
    {
	if (pthread_key_delete(threadSpecificKey) != 0)
	{
	    perror("pthread_key_delete");
	    goto error;
	}
	
	threadSpecificKey = -1;
    }

    free(dispatcher);

    refCounter--;

/* success: */
    pthread_mutex_unlock(&createAndDestroyLock);
    return 0;

error:
    pthread_mutex_unlock(&createAndDestroyLock);
    return -1;
}
