/*! ***************************************************************************
 * \file    egl.0.c
 * \brief   
 * 
 * \date    January 9, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <errno.h>
#include <uthash.h>
#include <stdio.h>
#include <EGL/egl.h>

#include "dispatcher.h"
#include "memory_manager.h"
#include "server.h"

#include "client/serializer.h"

struct ContextDispatcherMapping {
    void               *context;
    GVDISdispatcherptr  dispatcher;

    UT_hash_handle  hh;
};

extern char **environ;

static struct ContextDispatcherMapping *ctxDispMap = NULL;
static int                              initiated  = 0;
static int                              vmShmFd    = 0;
static int                              vmShmSize  = 0;

static int
createMapping(EGLContext context, GVDISdispatcherptr dispatcher)
{
    struct ContextDispatcherMapping *newMapping;

    if ((newMapping = malloc(sizeof(struct ContextDispatcherMapping))) == NULL)
    {
	perror("malloc");
	return -1;
    }	

    newMapping->context    = context;
    newMapping->dispatcher = dispatcher;

    HASH_ADD_PTR(ctxDispMap, context, newMapping);
    
    return 0;
}

static int
initEgl()
{
    GVTRPtransportptr  defaultTransport;
    GVDISdispatcherptr defaultDispatcher;
    size_t             offset;
    size_t             length;

    vmShmFd   = atoi(environ[0]);
    vmShmSize = atoi(environ[1]);

    length = 101 * 4096;

    if (vmShmSize < length)
    {
	errno = EINVAL;
	return -1;
    }

    if (gvmmgrAlloc(&offset, length) == -1)
    {
	perror("gvmmgrAlloc");
	return -1;
    }

    if (gvsrvHandshake(offset, length) == -1)
    {
	perror("gvsrvHandshake");
	return -1;
    }

    if (gvtrpCreate(&defaultTransport, &vmShmFd, offset, length) == -1)
    {
	perror("gvtrpCreate");
	return -1;
    }

    if (gvdisCreate(&defaultDispatcher, defaultTransport) == -1)
    {
	perror("gvdisCreate");
	return -1;
    }    

    if (gvdisMakeCurrent(defaultDispatcher) == -1)
    {
	perror("gvdisMakeCurrent");
    }

    if (createMapping((void *)0, defaultDispatcher) == -1)
    {
	return -1;
    }

    initiated = 1;

    return 0;
}

#define initIfNotDoneAlready()			\
    do {					\
	if (!initiated)				\
	{					\
	    if (initEgl() == -1) return -1;	\
	}					\
    } while (0)

EGLint
eglGetError()
{
    GVSERcallid callId;
    EGLint      error;

    initIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_GETERROR);
    gvserEndCall();

    gvserReturn(callId);
    gvserReturnValue(&error, sizeof(EGLint));
    gvserEndReturn();

    return error;
}
