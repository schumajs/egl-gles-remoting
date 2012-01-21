/*! ***************************************************************************
 * \file    server_state_tracker.0.c
 * \brief
 * 
 * \date    January 9, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <pthread.h>
#include <stdlib.h>

#include "error.h"
#include "process_state_map.h"
#include "server_state_tracker.0.h"
#include "thread_state_map.h"

/* ***************************************************************************
 * Server state tracker implementation
 */

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
gvDelOffsetState(size_t offset)
{
    TRY
    {
        if (pthread_rwlock_wrlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_wrlock");
        }

	if (gvDelProcessStateItem(offset) == -1)
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

GVoffsetstateptr
gvGetOffsetState(size_t offset)
{
    GVoffsetstateptr state;

    TRY
    {
        if (pthread_rwlock_rdlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_rdlock");
        }

	if ((state
	     = (GVoffsetstateptr) gvGetProcessStateItem(offset)) == NULL)
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
gvPutOffsetState(size_t           offset,
		 GVoffsetstateptr state)
{
    TRY
    {
        if (pthread_rwlock_wrlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_rdlock");
        }

	/* NOTE: heap manager guarantees that a particular offset is used only
	 * once at a time, so no collisions
	 */
	if (gvPutProcessStateItem(offset, state) == -1)
	{
	    THROW(e0, "gvPutProcessStateItem");
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

GVtransportptr
gvGetCurrentThreadTransport()
{
    GVtransportptr transport;

    initIfNotDoneAlready();

    TRY
    {
	if ((transport
	     = (GVtransportptr) gvGetThreadStateItem(
		 THREAD_TRANSPORT_KEY)) == NULL)
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
	if (gvPutThreadStateItem(THREAD_TRANSPORT_KEY, transport) == -1)
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
