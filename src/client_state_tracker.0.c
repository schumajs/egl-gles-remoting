/*! ***************************************************************************
 * \file    client_dispatcher.0.c
 * \brief
 * 
 * \date    January 9, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <uthash.h>

#include "client_state_tracker.h"
#include "error.h"
#include "thread_state.h"


struct ThreadState {
    /* EGL context */
    struct Context {
	EGLDisplay display;
        EGLDisplay context;
    } context;

    /* EGL context state */
    int            markedCurrent;
    int            markedDestroyed;

    /* Context transport */
    GVtransportptr transport;

    UT_hash_handle hh;
};

/* ***************************************************************************
 * Thread state hashtable
 */

static ThreadStatePtr   stateHash     = NULL;
static pthread_rwlock_t stateHashLock = PTHREAD_RWLOCK_INITIALIZER;

static void
hashDel(ThreadStatePtr state)
{
    HASH_DEL(stateHash, state);
}

static ThreadStatePtr
hashGet(EGLDisplay display,
	EGLContext context)
{
    struct ThreadState  pattern;
    struct ThreadState *state;

    memset(&pattern, 0, sizeof(struct ThreadState));
    pattern.context.display = display;
    pattern.context.context = context;

    HASH_FIND(hh, stateHash, &pattern.context, sizeof(struct Context), state);

    return state;
}

void
hashPut(ThreadStatePtr state) {
    HASH_ADD(hh, stateHash, context, sizeof(struct Context), state);
}

/* ****************************************************************************
 * Client state tracker implementation
 */

int
gvInitStateTracker()
{
    TRY
    {
	if (gvInitThreadState() == -1)
	{
	    THROW(e0, "gvInitThreadState");
	}
    }
    CATCH (e0)
    {
	return -1;
    }
   
    return 0;
}

int
gvTrack(EGLDisplay display,
	EGLContext context)
{
    ThreadStatePtr state;

    TRY
    {
	if (pthread_rwlock_wrlock(&stateHashLock) != 0)
	{
	    THROW(e0, "pthread_rwlock_wrlock");
	}

	if ((state = hashGet(display, context)) == NULL)
	{
	    if ((state = malloc(sizeof(struct ThreadState))) == NULL)
	    {
		THROW(e1, "malloc");
	    }

	    memset(&state, 0, sizeof(struct ThreadState));	

	    state->context.display = display;
	    state->context.context = context;	
	    state->markedCurrent   = 0;
	    state->markedDestroyed = 0;

	    hashPut(state);
	}
	else
	{
	    THROW(e1, "context already exists");
	}

	if (pthread_rwlock_unlock(&stateHashLock) != 0)
	{
	    THROW(e1, "pthread_rwlock_unlock");
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
	pthread_rwlock_unlock(&stateHashLock);
	return -1;
    }

    return 0;
}

int
gvUntrack(EGLDisplay display,
	  EGLContext context)
{
    ThreadStatePtr state;

    TRY
    {
	if (pthread_rwlock_wrlock(&stateHashLock) != 0)
	{
	    THROW(e0, "pthread_rwlock_wrlock");
	}

	if ((state = hashGet(display, context)) == NULL)
	{
	    hashDel(state);
	    free(state);
	}
	else
	{
	    THROW(e1, "no such context");
	}

	if (pthread_rwlock_unlock(&stateHashLock) != 0)
	{
	    THROW(e1, "pthread_rwlock_unlock");
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
	pthread_rwlock_unlock(&stateHashLock);
	return -1;
    }

    return 0;
}

int
gvIsMarkedCurrent(EGLDisplay display,
		  EGLContext context)
{
    ThreadStatePtr state;

    TRY
    {
	if (pthread_rwlock_rdlock(&stateHashLock) != 0)
	{
	    THROW(e0, "pthread_rwlock_rdlock");
	}

	if ((state = hashGet(display, context)) == NULL)
	{
	    THROW(e1, "no such context");
	}

	if (pthread_rwlock_unlock(&stateHashLock) != 0)
	{
	    THROW(e1, "pthread_rwlock_unlock");
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
	pthread_rwlock_unlock(&stateHashLock);
	return -1;
    }

    return state->markedCurrent;
}

int
gvMarkCurrent(EGLDisplay display,
	      EGLContext context)
{
    ThreadStatePtr state;

    TRY
    {
	if (pthread_rwlock_wrlock(&stateHashLock) != 0)
	{
	    THROW(e0, "pthread_rwlock_wrlock");
	}

	if ((state = hashGet(display, context)) == NULL)
	{
	    THROW(e1, "no such context");
	}

	state->markedCurrent = 1;

	if (pthread_rwlock_unlock(&stateHashLock) != 0)
	{
	    THROW(e1, "pthread_rwlock_unlock");
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
	pthread_rwlock_unlock(&stateHashLock);
	return -1;
    }

    return 0;
}

int
gvIsMarkedDestroyed(EGLDisplay display,
		    EGLContext context)
{
    ThreadStatePtr state;

    TRY
    {
	if (pthread_rwlock_rdlock(&stateHashLock) != 0)
	{
	    THROW(e0, "pthread_rwlock_rdlock");
	}

	if ((state = hashGet(display, context)) == NULL)
	{
	    THROW(e1, "no such context");
	}

	if (pthread_rwlock_unlock(&stateHashLock) != 0)
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
	pthread_rwlock_unlock(&stateHashLock);
	return -1;
    }

    return state->markedDestroyed;
}

int
gvMarkDestroyed(EGLDisplay display,
		EGLContext context)
{
    ThreadStatePtr state;

    TRY
    {
	if (pthread_rwlock_wrlock(&stateHashLock) != 0)
	{
	    THROW(e0, "pthread_rwlock_wrlock");
	}

	if ((state = hashGet(display, context)) == NULL)
	{
	    THROW(e1, "no such context");
	}

	state->markedDestroyed = 1;

	if (pthread_rwlock_unlock(&stateHashLock) != 0)
	{
	    THROW(e1, "pthread_rwlock_unlock");
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
	pthread_rwlock_unlock(&stateHashLock);
	return -1;
    }

    return 0;
}

int
gvGetCurrent(EGLDisplay     *display,
	     EGLContext     *context,
	     GVtransportptr *transport)
{
    ThreadStatePtr state; 

    TRY
    {
	if (gvGetThreadState(&state) == -1)
	{
	    THROW(e0, "no current context");
	}

	*display   = state->context.display;
	*context   = state->context.context;
	*transport = state->transport;
    }
    CATCH(e0)
    {
	return -1;
    }

    return 0;
}

int
gvSetCurrent(EGLDisplay     display,
	     EGLContext     context,
	     GVtransportptr transport)
{
    ThreadStatePtr state;

    TRY
    {
	if (pthread_rwlock_wrlock(&stateHashLock) != 0)
	{
	    THROW(e0, "pthread_rwlock_wrlock");
	}

	if ((state = hashGet(display, context)) == NULL)
	{
	    THROW(e1, "no such context");	    
	}

	state->markedCurrent = 1;
	state->transport     = transport;

	if (gvSetThreadState(state) == -1)
	{
	    THROW(e1, "gvSetThreadState");
	}

	if (pthread_rwlock_unlock(&stateHashLock) != 0)
	{
	    THROW(e1, "pthread_rwlock_unlock");
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
	pthread_rwlock_unlock(&stateHashLock);
	return -1;
    }

    return 0;
}

int
gvTermStateTracker()
{
    TRY
    {
	if (gvTermThreadState() == -1)
	{
	    THROW(e0, "gvInitThreadState");
	}
    }
    CATCH (e0)
    {
	return -1;
    }
   
    return 0;
}
