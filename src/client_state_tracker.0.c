/*! ***************************************************************************
 * \file    client_state_tracker.0.c
 * \brief
 * 
 * \date    January 9, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <pthread.h>

#include "client_state_tracker.0.h"
#include "error.h"
#include "process_state_map.h"
#include "thread_state_map.h"

/* ***************************************************************************
 * Client state tracker implementation
 */

#define CURRENT_DISPLAY_KEY   0x0
#define CURRENT_CONTEXT_KEY   0x0
#define CURRENT_TRANSPORT_KEY 0x1

static pthread_rwlock_t processStateLock        = PTHREAD_RWLOCK_INITIALIZER;
static int              threadStateMapInitiated = 0;

#define initIfNotDoneAlready()					\
    do {							\
	if (!threadStateMapInitiated)				\
	{							\
	    if (gvInitThreadStateMap() == -1) return -1;	\
	    threadStateMapInitiated = 1;			\
	}							\
    } while (0)

struct ForeachArg {
    EGLDisplay            display;
    GVforeachcontextfunc  func;
    void                 *arg;
};

static void
foreachContextStateCallback(unsigned long int  key,
			    void              *value,
			    void              *arg)
{
    struct ForeachArg *foreachArg = (struct ForeachArg *)arg;
    GVcontextstateptr  state      = (GVcontextstateptr)value;
    
    if (state->display == foreachArg->display)
    {
	(foreachArg->func)(state, foreachArg->arg);
    }
}

static void
setAllMarkedDestroyedCallback(GVcontextstateptr  state,
			      void              *arg)
{
    state->markedDestroyed = *((int *)arg);
}

int
gvDelEglContextState(EGLDisplay display,
		     EGLContext context)
{
    TRY
    {
        if (pthread_rwlock_wrlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_wrlock");
        }

	if (gvDelProcessStateItem((size_t) context) == -1)
	{
	    THROW(e1, "gvDelProcessStateItem");
	}

        if (pthread_rwlock_unlock(&processStateLock) != 0)
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
        pthread_rwlock_unlock(&processStateLock);
        return -1;
    }

    return 0;
}

GVcontextstateptr
gvGetEglContextState(EGLDisplay display,
		     EGLContext context)
{
    GVcontextstateptr state;

    TRY
    {
        if (pthread_rwlock_rdlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_rdlock");
        }

	if ((state
	     = (GVcontextstateptr) gvGetProcessStateItem(
		 (size_t) context)) == NULL)
	{
	    THROW(e1, "gvGetProcessStateItem");
	}

        if (pthread_rwlock_unlock(&processStateLock) != 0)
        {
            THROW(e1, "pthread_rwlock_unlock");
        }
    }
    CATCH (e0)
    {
	return NULL;
    }
    CATCH (e1)
    {
        /* Try to unlock. At this point we can't do anything if unlocking
         * fails, so just ignore errors.
         */
        pthread_rwlock_unlock(&processStateLock);
        return NULL;
    }

    return state;
}

int
gvForeachEglContextState(EGLDisplay            display,
			 GVforeachcontextfunc  func,
			 void                 *arg)
{
    struct ForeachArg foreachArg;

    TRY
    {
        if (pthread_rwlock_wrlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_wrlock");
        }

	foreachArg.display = display;
	foreachArg.func    = func;
	foreachArg.arg     = arg;

	if (gvForeachProcessStateItem(foreachContextStateCallback,
				      &foreachArg) == -1)
	{
	    THROW(e1, "gvForeachProcessStateItem");
	}

        if (pthread_rwlock_unlock(&processStateLock) != 0)
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
        pthread_rwlock_unlock(&processStateLock);
        return -1;
    }

    return 0;
}

int
gvPutEglContextState(EGLDisplay        display,
		     EGLContext        context,
		     GVcontextstateptr state)
{
    TRY
    {
        if (pthread_rwlock_wrlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_wrlock");
        }

	if (gvPutProcessStateItem((size_t) context, state) == -1)
	{ 
	    THROW(e1, "gvPutProcessStateItem");
	}

        if (pthread_rwlock_unlock(&processStateLock) != 0)
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
        pthread_rwlock_unlock(&processStateLock);
        return -1;
    }

    return 0;
}

int
gvIsMarkedCurrent(EGLDisplay display,
                  EGLContext context)
{
    int               markedCurrent;
    GVcontextstateptr state;

    TRY
    {
        if (pthread_rwlock_rdlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_rdlock");
        }

	if ((state
	     = (GVcontextstateptr) gvGetProcessStateItem(
		 (size_t) context)) == NULL)
	{
	    THROW(e1, "gvGetProcessStateItem");
	}

	markedCurrent = state->markedCurrent;

        if (pthread_rwlock_unlock(&processStateLock) != 0)
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
        pthread_rwlock_unlock(&processStateLock);
        return -1;
    }

    return markedCurrent;
}

int gvSetMarkedCurrent(EGLDisplay display,
		       EGLContext context,
		       int        markedCurrent)
{
    GVcontextstateptr state;

    TRY
    {
        if (pthread_rwlock_wrlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_wrlock");
        }

	if ((state
	     = (GVcontextstateptr) gvGetProcessStateItem(
		 (size_t) context)) == NULL)
	{
	    THROW(e1, "gvGetProcessStateItem");
	}

        state->markedCurrent = markedCurrent;

        if (pthread_rwlock_unlock(&processStateLock) != 0)
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
        pthread_rwlock_unlock(&processStateLock);
        return -1;
    }

    return 0;
}

int
gvIsMarkedDestroyed(EGLDisplay display,
                    EGLContext context)
{
    int               markedDestroyed;
    GVcontextstateptr state;

    TRY
    {
        if (pthread_rwlock_rdlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_rdlock");
        }

	if ((state
	     = (GVcontextstateptr) gvGetProcessStateItem(
		 (size_t) context)) == NULL)
	{
	    THROW(e1, "gvGetProcessStateItem");
	}

	markedDestroyed = state->markedDestroyed;

        if (pthread_rwlock_unlock(&processStateLock) != 0)
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
        pthread_rwlock_unlock(&processStateLock);
        return -1;
    }

    return markedDestroyed;
}

int
gvSetMarkedDestroyed(EGLDisplay display,
		     EGLContext context,
		     int        markedDestroyed)
{
    GVcontextstateptr state;

    TRY
    {
        if (pthread_rwlock_wrlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_wrlock");
        }

	if ((state
	     = (GVcontextstateptr) gvGetProcessStateItem(
		 (size_t) context)) == NULL)
	{
	    THROW(e1, "gvGetProcessStateItem");
	}

        state->markedDestroyed = markedDestroyed;

        if (pthread_rwlock_unlock(&processStateLock) != 0)
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
        pthread_rwlock_unlock(&processStateLock);
        return -1;
    }

    return 0;
}

int
gvSetAllMarkedDestroyed(EGLDisplay display,
			int        markedDestroyed)
{
    TRY
    {
	if (gvForeachEglContextState(display,
				     setAllMarkedDestroyedCallback,
				     &markedDestroyed) == -1)
	{
	    THROW(e0, "gvForeachEglContextState");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

EGLDisplay
gvGetCurrentThreadDisplay()
{
    EGLDisplay display;

    initIfNotDoneAlready();

    TRY
    {
	if ((display
	     = (EGLDisplay) gvGetThreadStateItem(CURRENT_DISPLAY_KEY)) == NULL)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return display;
}

int
gvSetCurrentThreadDisplay(EGLDisplay display)
{
    initIfNotDoneAlready();

    TRY
    {
	/* NOTE: only set once per thread in concept 0, so no collisions */
	if (gvPutThreadStateItem(CURRENT_DISPLAY_KEY, display) == -1)
	{
	    THROW(e0, "gvPutThreadStateItem");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

EGLContext
gvGetCurrentThreadContext()
{
    EGLContext context;

    initIfNotDoneAlready();

    TRY
    {
	if ((context
	     = (EGLContext) gvGetThreadStateItem(CURRENT_CONTEXT_KEY)) == NULL)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return context;
}

int
gvSetCurrentThreadContext(EGLContext context)
{
    initIfNotDoneAlready();

    TRY
    {
	/* NOTE: only set once per thread in concept 0, so no collisions */
	if (gvPutThreadStateItem(CURRENT_CONTEXT_KEY, context) == -1)
	{
	    THROW(e0, "gvPutThreadStateItem");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

GVtransportptr
gvGetCurrentThreadTransport(void)
{
    GVtransportptr transport;

    initIfNotDoneAlready();

    TRY
    {
	if ((transport
	     = (GVtransportptr) gvGetThreadStateItem(
		 CURRENT_TRANSPORT_KEY)) == NULL)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return transport;
}

int
gvSetCurrentThreadTransport(GVtransportptr transport)
{
    initIfNotDoneAlready();

    TRY
    {
	/* NOTE: only set once per thread in concept 0, so no collisions */
	if (gvPutThreadStateItem(CURRENT_TRANSPORT_KEY, transport) == -1)
	{
	    THROW(e0, "gvPutThreadStateItem");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}
