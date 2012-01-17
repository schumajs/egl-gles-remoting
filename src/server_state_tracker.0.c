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

struct JanitorState {
    pthread_t      thread;
    GVtransportptr transport;
};

static int currentThreadTransportKey = 0;

int
gvGetCurrentThreadTransport(GVtransportptr *transport)
{
    void *tempTransport;

    TRY
    {
	if (gvGetThreadState(&currentThreadTransportKey,
			     sizeof(int),
			     &tempTransport) == -1)
	{
	    THROW(e0, "gvPutThreadState");
	}

	*transport = tempTransport;
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvSetCurrentThreadTransport(GVtransportptr transport)
{
    TRY
    {
	/* NOTE: only set once in concept 0, so no collisions */
	if (gvPutThreadState(&currentThreadTransportKey,
			     sizeof(int),
			     transport) == -1)
	{
	    THROW(e0, "gvPutThreadState");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvDelJanitorState(size_t offset)
{
    void *tempState; 

    TRY
    {
	if (gvGetProcessState(&offset,
			      sizeof(size_t),
			      &tempState) == -1)
	{
	    THROW(e0, "gvGetProcessState");
	}

	if (gvDelProcessState(&offset, sizeof(size_t)) == -1)
	{
	    THROW(e0, "gvDelProcessState");
	}

	free(tempState);
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvGetJanitorState(size_t          offset,
		  pthread_t      *thread,
		  GVtransportptr *transport)
{
    struct JanitorState *state;
    void                *tempState;

    TRY
    {
	puts("AA");

	if (gvGetProcessState(&offset,
			      sizeof(size_t),
			      &tempState) == -1)
	{
	    THROW(e0, "gvGetProcessState");
	}

	puts("AB");

	state = (struct JanitorState *)tempState;

	*thread    = state->thread;
	*transport = state->transport;
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvPutJanitorState(size_t         offset,
		  pthread_t      thread,
		  GVtransportptr transport)
{
    struct JanitorState *state;

    TRY
    {
	if ((state = malloc(sizeof(struct JanitorState))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	state->thread    = thread;
	state->transport = transport;

	/* NOTE: heap manager guarantees that a particular offset is used only
	 * once at a time, so no collisions
	 */
	if (gvPutProcessState(&offset,
			      sizeof(size_t),
			      state) == -1)
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
