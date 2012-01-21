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

	if (gvSetCurrentThreadTransport(defaultTransport) == -1)
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

	if (gvPutEglContextState(DEFAULT_DISPLAY,
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
	if (gvGetCurrentThreadTransport() == NULL)			\
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
	    if (gvSetCurrentThreadTransport(state->transport) == -1)	\
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

    transport = gvGetCurrentThreadTransport();

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

    transport = gvGetCurrentThreadTransport();

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

    transport = gvGetCurrentThreadTransport();

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

    transport = gvGetCurrentThreadTransport();

    callId = gvStartSending(transport, NULL, GV_CMDID_EGL_TERMINATE);
    gvSendData(transport, &display, sizeof(EGLDisplay));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, &status, sizeof(EGLBoolean));

    // TODO destroy contexts

    return status;
}

const char
*eglQueryString(EGLDisplay display,
		EGLint     name)
{
    GVtransportptr  transport;

    GVcallid        callId;
    char          * queryString;
    size_t          queryStringLength;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

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

EGLBoolean
eglGetConfigs(EGLDisplay display,
	      EGLConfig *configs,
	      EGLint     configSize,
	      EGLint    *numConfig)
{
    GVtransportptr  transport;

    GVcallid        callId;
    EGLBoolean      status;
    int             configsNull = (configs == NULL);

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    callId = gvStartSending(transport, NULL, GV_CMDID_EGL_GETCONFIGS);
    gvSendData(transport, &display, sizeof(EGLDisplay));
    gvSendData(transport, &configsNull, sizeof(int));
    gvSendData(transport, &configSize, sizeof(EGLint));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, &status, sizeof(EGLBoolean));
    gvReceiveData(transport, numConfig, sizeof(EGLint));
    /* NOTE configs has to be preallocated. If configs == NULL then no configs
     * are returned, only numConfig is returned (see spec. p. 23)
     */
    if (!configsNull)
    {
        gvReceiveData(transport, configs, *numConfig * sizeof(EGLConfig));
    }

    return status;
}

EGLBoolean
eglChooseConfig(EGLDisplay    display,
		const EGLint *attribList,
		EGLConfig    *configs,
		EGLint        configSize,
		EGLint       *numConfig)
{
    GVtransportptr  transport;

    GVcallid        callId;
    EGLBoolean      status;
    int             attribListSize;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    getAttribListSize(attribList, attribListSize);

    callId = gvStartSending(transport, NULL, GV_CMDID_EGL_CHOOSECONFIG);
    gvSendData(transport, &display, sizeof(EGLDisplay));
    gvSendData(transport, &attribListSize, sizeof(int));
    gvSendData(transport, attribList, attribListSize * sizeof(EGLint));
    gvSendData(transport, &configSize, sizeof(EGLint));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, &status, sizeof(EGLBoolean));
    gvReceiveData(transport, numConfig, sizeof(EGLint));
    /* NOTE configs has to be preallocated, configs == NULL undefined ... */
    gvReceiveData(transport, configs, *numConfig * sizeof(EGLConfig));

    return status;
}

EGLBoolean
eglGetConfigAttrib(EGLDisplay  display,
		   EGLConfig   config,
		   EGLint      attribute,
		   EGLint     *value)
{
    GVtransportptr  transport;

    GVcallid        callId;
    EGLBoolean      status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    callId = gvStartSending(transport, NULL, GV_CMDID_EGL_CHOOSECONFIG);
    gvSendData(transport, &display, sizeof(EGLDisplay));
    gvSendData(transport, &config, sizeof(EGLConfig));
    gvSendData(transport, &attribute, sizeof(EGLint));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, &status, sizeof(EGLBoolean));
    gvReceiveData(transport, value, sizeof(EGLint));

    return status;
}

EGLSurface
eglCreateWindowSurface(EGLDisplay           display,
		       EGLConfig            config,
		       EGLNativeWindowType  window,
		       const EGLint        *attribList)
{
    GVtransportptr  transport;

    GVcallid        callId;
    EGLSurface      surface;
    int             attribListSize;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    getAttribListSize(attribList, attribListSize);

    callId = gvStartSending(transport, NULL, GV_CMDID_EGL_CREATEWINDOWSURFACE);
    gvSendData(transport, &display, sizeof(EGLDisplay));
    gvSendData(transport, &config, sizeof(EGLConfig));
    gvSendData(transport, &window, sizeof(EGLNativeWindowType));
    gvSendData(transport, &attribListSize, sizeof(int));
    gvSendData(transport, attribList, attribListSize * sizeof(EGLint));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, &surface, sizeof(EGLSurface));

    return surface;
}

EGLSurface
eglCreatePbufferSurface(EGLDisplay    display,
			EGLConfig     config,
			const EGLint *attribList)
{
    return NULL;
}

EGLSurface
eglCreatePixmapSurface(EGLDisplay           display,
		       EGLConfig            config,
                       EGLNativePixmapType  pixmap,
		       const EGLint        *attribList)
{
    return NULL;
}

EGLBoolean
eglDestroySurface(EGLDisplay display,
		  EGLSurface surface)
{
    return 1;
}

EGLBoolean 
eglQuerySurface(EGLDisplay  display,
		EGLSurface  surface,
		EGLint      attribute,
		EGLint     *value)
{
    return 1;
}

EGLBoolean
eglBindAPI(EGLenum api)
{
    GVtransportptr  transport;

    GVcallid        callId;
    EGLBoolean      status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    callId = gvStartSending(transport, NULL, GV_CMDID_EGL_BINDAPI);
    gvSendData(transport, &api, sizeof(EGLenum));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, &status, sizeof(EGLBoolean));

    return status;
}

EGLenum
eglQueryAPI()
{
    return 0;
}

EGLBoolean
eglWaitClient()
{
    return 1;
}

EGLBoolean
eglReleaseThread(void)
{
    return 1;
}

EGLSurface
eglCreatePbufferFromClientBuffer(EGLDisplay       display,
				 EGLenum          bufferType,
				 EGLClientBuffer  buffer,
				 EGLConfig        config,
				 const EGLint    *attribList)
{
    return NULL;
}

EGLBoolean
eglSurfaceAttrib(EGLDisplay display,
		 EGLSurface surface,
		 EGLint     attribute,
		 EGLint     value)
{
    return 1;
}

EGLBoolean
eglBindTexImage(EGLDisplay display,
		EGLSurface surface,
		EGLint     buffer)
{
    return 1;
}

EGLBoolean
eglReleaseTexImage(EGLDisplay display,
		   EGLSurface surface,
		   EGLint     buffer)
{
    return 1;
}

EGLBoolean
eglSwapInterval(EGLDisplay display,
		EGLint     interval)
{
    return 1;
}

EGLContext
eglCreateContext(EGLDisplay    display,
		 EGLConfig     config,
		 EGLContext    shareContext,
		 const EGLint *attribList)
{
    return NULL;
}

EGLBoolean
eglDestroyContext(EGLDisplay display,
		  EGLContext context)
{
    return 1;
}

EGLBoolean
eglMakeCurrent(EGLDisplay display,
	       EGLSurface drawSurface,
	       EGLSurface readSurface,
	       EGLContext context)
{
    return 1;
}

EGLContext
eglGetCurrentContext()
{
    return NULL;
}

EGLSurface
eglGetCurrentSurface(EGLint readdraw)
{
    return NULL;
}

EGLDisplay
eglGetCurrentDisplay()
{
    return NULL;
}

EGLBoolean
eglQueryContext(EGLDisplay  display,
		EGLContext  context,
		EGLint      attribute,
		EGLint     *value)
{
    return 1;
}

EGLBoolean
eglWaitGL()
{
    return 1;
}

EGLBoolean
eglWaitNative(EGLint engine)
{
    return 1;
}

EGLBoolean
eglSwapBuffers(EGLDisplay display,
	       EGLSurface surface)
{
    return 1;
}

EGLBoolean
eglCopyBuffers(EGLDisplay          display,
	       EGLSurface          surface,
	       EGLNativePixmapType target)
{
    return 1;
}
