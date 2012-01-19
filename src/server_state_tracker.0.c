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

#define DISPATCHER_STATE_KEY 0

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
gvDelJanitorState(size_t offset)
{
    TRY
    {
	if (gvDelProcessState(offset) == -1)
	{
	    THROW(e0, "gvDelProcessState");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvGetJanitorState(size_t             offset,
		  GVjanitorstateptr *state)
{
    void *tempState;

    TRY
    {
	if (gvGetProcessState(offset, &tempState) == -1)
	{
	    THROW(e0, "gvGetProcessState");
	}

	*state = (GVjanitorstateptr) tempState;
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvSetJanitorState(size_t            offset,
		  GVjanitorstateptr state)
{
    TRY
    {
	/* NOTE: heap manager guarantees that a particular offset is used only
	 * once at a time, so no collisions
	 */
	if (gvPutProcessState(offset, state) == -1)
	{
	    THROW(e0, "gvPutProcessState");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvDelDispatcherState(GVdispatcherstateptr state)
{
    initIfNotDoneAlready();

    TRY
    {
	if (gvDelThreadState(DISPATCHER_STATE_KEY) == -1)
	{
	    THROW(e0, "gvDelThreadState");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvGetDispatcherState(GVdispatcherstateptr *state)
{
    void *tempState;

    initIfNotDoneAlready();

    TRY
    {
	if (gvGetThreadState(DISPATCHER_STATE_KEY, &tempState) == -1)
	{
	    THROW(e0, "gvGetThreadState");
	}

	*state = tempState;
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvSetDispatcherState(GVdispatcherstateptr state)
{
    initIfNotDoneAlready();

    TRY
    {
	/* NOTE: only set once per thread in concept 0, so no collisions */
	if (gvPutThreadState(DISPATCHER_STATE_KEY, state) == -1)
	{
	    THROW(e0, "gvSetThreadState");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}