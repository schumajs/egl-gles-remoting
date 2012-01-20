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

#define _MULTI_THREADED
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <EGL/egl.h>

#include "client_serializer.h"
#include "client_state_tracker.0.h"
#include "error.h"
#include "heap_manager.h"
#include "janitor.h"

#define CONTEXT_TRANSPORT_LENGTH 11 * 4096

#define DEFAULT_DISPLAY (void *)0
#define DEFAULT_CONTEXT (void *)0

extern char            **environ;

static GVshmptr          vmShm;

static int               processInitiated = 0;
static pthread_mutex_t   initProcessLock  = PTHREAD_MUTEX_INITIALIZER;

static int
initEglGlesClient()
{
    GVtransportptr    defaultTransport;
    size_t            defaultTransportOffset;
    GVcontextstateptr eglContextState;
    int               vmShmFd   = atoi(environ[0]);
    int               vmShmSize = atoi(environ[1]);

    TRY
    {
	if ((vmShm = malloc(sizeof(struct GVshm))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	vmShm->id   = vmShmFd;
	vmShm->size = vmShmSize;

	if (vmShmSize < CONTEXT_TRANSPORT_LENGTH)
	{
	    errno = EINVAL;
	    THROW(e0, "not enough space");
	}

	if ((defaultTransportOffset = gvAlloc(CONTEXT_TRANSPORT_LENGTH)) == 0)
	{
	    THROW(e0, "gvAlloc");
	}

	if (gvBonjour(defaultTransportOffset, CONTEXT_TRANSPORT_LENGTH) == -1)
	{
	    THROW(e0, "gvBonjour");
	}

	if ((defaultTransport
	     = gvCreateTransport(vmShm,
				 defaultTransportOffset,
				 CONTEXT_TRANSPORT_LENGTH)) == NULL)
	{
	    THROW(e0, "gvCreateTransport");
	}

	if (gvSetThreadTransport(defaultTransport) == -1)
	{
	    THROW(e0, "gvSetThreadTransport");
	}

	if ((eglContextState = malloc(sizeof(struct GVcontextstate))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	eglContextState->markedCurrent   = 1;
	eglContextState->markedDestroyed = 0;
	eglContextState->transport       = defaultTransport;

	if (gvSetEglContextState(DEFAULT_DISPLAY,
				 DEFAULT_CONTEXT,
				 eglContextState) == -1)
	{
	    THROW(e0, "gvSetDispatcherState");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

/* TODO are there any issues with double checked locking? */
#define initProcessIfNotDoneAlready()				\
    do {							\
        if (!processInitiated)					\
        {							\
            pthread_mutex_lock(&initProcessLock);		\
            if (!processInitiated)				\
            {							\
	        if (initEglGlesClient() == -1)			\
	        {						\
	            pthread_mutex_unlock(&initProcessLock);	\
	            /* TODO ? */				\
		    exit(2);					\
	        }						\
		processInitiated = 1;				\
            }							\
            pthread_mutex_unlock(&initProcessLock);		\
        }							\
    } while (0)

#define initThreadIfNotDoneAlready()					\
    do {								\
	if (gvGetThreadTransport() == NULL)				\
	{								\
	    GVcontextstateptr state;					\
	    								\
	    if ((state							\
		 = gvGetEglContextState(DEFAULT_DISPLAY,		\
					DEFAULT_CONTEXT)) == NULL)	\
	    {								\
		/* TODO ? */						\
                exit(2);						\
	    }								\
									\
	    if (gvSetThreadTransport(state->transport) == -1)		\
	    {								\
		exit(2);						\
	    }								\
	}								\
    } while (0)

/* ***************************************************************************
 * EGL
 */

/* NOTE: attribList is terminated with EGL_NONE, so assert: min.
   attribListSize == 1
*/
#define getAttribListSize(attribList, attribListSize)		\
    do {							\
	attribListSize = 0;					\
	if (attribList != NULL)					\
	{							\
	    while (attribList[attribListSize] != EGL_NONE)	\
	    {							\
		attribListSize++;				\
	    }							\
	}							\
    } while (0)

EGLint
eglGetError()
{
    GVtransportptr transport;

    GVcallid       callId;
    EGLint         error;
  
    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetThreadTransport();

    callId = gvStartSending(transport, NULL, GV_CMDID_EGL_GETERROR);

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, &error, sizeof(EGLint));

    return error;
}

EGLDisplay
eglGetDisplay(EGLNativeDisplayType displayId)
{
    GVtransportptr transport;

    GVcallid       callId;
    EGLDisplay     display;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetThreadTransport();

    callId = gvStartSending(transport, NULL, GV_CMDID_EGL_GETDISPLAY);
    gvSendData(transport, &displayId, sizeof(EGLNativeDisplayType));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, &display, sizeof(EGLDisplay));

    return display;
}

EGLBoolean
eglInitialize(EGLDisplay  display,
	      EGLint     *major,
	      EGLint     *minor)
{
    GVtransportptr transport;

    GVcallid       callId;
    EGLint         status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetThreadTransport();

    callId = gvStartSending(transport, NULL, GV_CMDID_EGL_INITIALIZE);
    gvSendData(transport, &display, sizeof(EGLDisplay));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, &status, sizeof(EGLBoolean));
    gvReceiveData(transport, major, sizeof(EGLint));
    gvReceiveData(transport, minor, sizeof(EGLint));

    return status;
}

EGLBoolean
eglTerminate(EGLDisplay display)
{
    GVtransportptr transport;

    GVcallid       callId;
    EGLint         status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvStartSending(transport, NULL, GV_CMDID_EGL_TERMINATE);
    gvSendData(transport, &display, sizeof(EGLDisplay));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, &status, sizeof(EGLBoolean));

    // TODO destroy contexts

    return status;
}

const char
*eglQueryString(EGLDisplay display, EGLint name)
{
    GVtransportptr  transport;

    GVcallid        callId;
    char          * queryString;
    size_t          queryStringLength;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvStartSending(transport, NULL, GV_CMDID_EGL_QUERYSTRING);
    gvSendData(transport, &display, sizeof(EGLDisplay));
    gvSendData(transport, &name, sizeof(EGLint));

    gvStartReceiving(transport, NULL, callId);
    /* Read string length: strlen(queryString) + 1 */
    gvReceiveData(transport, &queryStringLength, sizeof(size_t));
    /* Read actual string */
    queryString = malloc(queryStringLength * sizeof(char));
    gvReceiveData(transport, queryString, queryStringLength * sizeof(char));

    return queryString;
}
