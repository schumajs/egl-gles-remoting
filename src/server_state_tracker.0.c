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

#include <stdlib.h>

#include "error.h"
#include "process_state_map.h"
#include "server_state_tracker.0.h"
#include "thread_state_map.h"

/* ***************************************************************************
 * Server state tracker implementation
 */

#define THREAD_TRANSPORT_KEY 0

static int threadStateMapInitiated = 0;

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
	if (gvDelProcessItem(offset) == -1)
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

GVoffsetstateptr
gvGetOffsetState(size_t offset)
{
    void *tempState;

    TRY
    {
	if ((tempState = gvGetProcessItem(offset)) == NULL)
	{
	    THROW(e0, "gvGetProcessItem");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return (GVoffsetstateptr) tempState;
}

int
gvSetOffsetState(size_t           offset,
		 GVoffsetstateptr state)
{
    TRY
    {
	/* NOTE: heap manager guarantees that a particular offset is used only
	 * once at a time, so no collisions
	 */
	if (gvPutProcessItem(offset, state) == -1)
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
gvGetThreadTransport()
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
