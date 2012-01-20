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

/* TODO replace cantor pairing: slow, needs bignum */
#define getCantorPair(k1, k2) \
    0.5 * ((k1 + k2) * (k1 + k2 + 1) + k2)

#define THREAD_TRANSPORT_KEY 0

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

int
gvDelEglContextState(EGLDisplay display,
		     EGLContext context)
{
    size_t tempDisplay = (size_t) display;
    size_t tempContext = (size_t) context;

    TRY
    {
	if (gvDelProcessItem(getCantorPair(tempDisplay,
					   tempContext)) == -1)
	{
	    THROW(e0, "gvDelProcessItem");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

GVcontextstateptr
gvGetEglContextState(EGLDisplay display,
		     EGLContext context)
{
    size_t  tempDisplay = (size_t) display;
    size_t  tempContext = (size_t) context;
    void   *tempState;

    TRY
    {
	if ((tempState
	     = gvGetProcessItem(getCantorPair(tempDisplay,
					      tempContext))) == NULL)
	{
	    THROW(e0, "gvGetProcessItem");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return (GVcontextstateptr) tempState;
}

int
gvSetEglContextState(EGLDisplay        display,
		     EGLContext        context,
		     GVcontextstateptr state)
{
    size_t  tempDisplay = (size_t) display;
    size_t  tempContext = (size_t) context;

    TRY
    {
	if (gvPutProcessItem(getCantorPair(tempDisplay,
					   tempContext), state) == -1)
	{
	    THROW(e0, "gvPutProcessItem");
	}
    }
    CATCH (e0)
    {
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

        if ((state = gvGetEglContextState(display, context)) == NULL)
        {
            THROW(e1, "no such context");
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

int gvSetMarkCurrent(EGLDisplay display,
		     EGLContext context)
{
    GVcontextstateptr state;

    TRY
    {
        if (pthread_rwlock_wrlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_wrlock");
        }

        if ((state = gvGetEglContextState(display, context)) == NULL)
        {
            THROW(e1, "no such context");
        }

        state->markedCurrent = 1;

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

        if ((state = gvGetEglContextState(display, context)) == NULL)
        {
            THROW(e1, "no such context");
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
gvSetMarkDestroyed(EGLDisplay display,
		   EGLContext context)
{
    GVcontextstateptr state;

    TRY
    {
        if (pthread_rwlock_wrlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_wrlock");
        }

        if ((state = gvGetEglContextState(display, context)) == NULL)
        {
            THROW(e1, "no such context");
        }

        state->markedDestroyed = 1;

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
gvDelThreadTransport()
{
    initIfNotDoneAlready();

    TRY
    {
	if (gvDelThreadItem(THREAD_TRANSPORT_KEY) == -1)
	{
	    THROW(e0, "gvDelThreadItem");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

GVtransportptr
gvGetThreadTransport(void)
{
    void *tempTransport;

    initIfNotDoneAlready();

    TRY
    {
	if ((tempTransport = gvGetThreadItem(THREAD_TRANSPORT_KEY)) == NULL)
	{
	    THROW(e0, "gvGetThreadItem");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return (GVtransportptr) tempTransport;
}

int
gvSetThreadTransport(GVtransportptr transport)
{
    initIfNotDoneAlready();

    TRY
    {
	/* NOTE: only set once per thread in concept 0, so no collisions */
	if (gvPutThreadItem(THREAD_TRANSPORT_KEY, transport) == -1)
	{
	    THROW(e0, "gvPutThreadItem");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

