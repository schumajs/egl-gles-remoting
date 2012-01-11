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

    UT_hash_handle hh;
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
initEglGles()
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
	    if (initEglGles() == -1) return -1;	\
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
    gvserOutData(&error, sizeof(EGLint));
    gvserEndReturn();

    return error;
}

EGLDisplay
eglGetDisplay(EGLNativeDisplayType displayId)
{
    GVSERcallid callId;
    EGLDisplay  display;

    initIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_GETDISPLAY);
    gvserInData(&displayId, sizeof(EGLNativeDisplayType));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&display, sizeof(EGLDisplay));
    gvserEndReturn();

    return display;
}

EGLBoolean
eglInitialize(EGLDisplay display, EGLint *major, EGLint *minor)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_INITIALIZE);
    gvserInData(&display, sizeof(EGLDisplay));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserOutData(major, sizeof(EGLint));
    gvserOutData(minor, sizeof(EGLint));
    gvserEndReturn();

    return status;
}

/*
 * TODO destroy resources if last display is terminated. What to do in case of
 * error?
 */
EGLBoolean
eglTerminate(EGLDisplay display)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_TERMINATE);
    gvserInData(&display, sizeof(EGLDisplay));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserEndReturn();

    return status;
}

const char
*eglQueryString(EGLDisplay display, EGLint name)
{
    GVSERcallid  callId;
    char        *queryString;
    int          queryStringLength;

    initIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_QUERYSTRING);
    gvserInData(&display, sizeof(EGLDisplay));
    gvserInData(&name, sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(queryString, sizeof(char));
    gvserEndReturn();

    return queryString;
}

EGLBoolean
eglGetConfigs(EGLDisplay display, EGLConfig *configs, EGLint configSize, EGLint *numConfig)
{
    return 1;
}
