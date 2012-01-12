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
#include <uthash.h>
#include <stdio.h>
#include <EGL/egl.h>

#include "dispatcher.h"
#include "memory_manager.h"
#include "server.h"

#include "client/serializer.h"

extern char **environ;

/* ***************************************************************************
 * ctxDispMap
 */

struct Key {
    EGLDisplay display;
    EGLContext context;
};

struct Element {
    struct Key         key;
    GVDISdispatcherptr dispatcher;
    int                markedForDeletion;

    UT_hash_handle     hh;
};

static struct Element    *ctxDispMap     = NULL;
static pthread_rwlock_t   ctxDispMapLock = PTHREAD_RWLOCK_INITIALIZER;

static void
del(EGLDisplay display, EGLContext context)
{
    struct Element *element;
    struct Element  pattern;

    memset(&pattern, 0, sizeof(struct Element));
    pattern.key.display = display;
    pattern.key.context = context;

    pthread_rwlock_wrlock(&ctxDispMapLock);
    HASH_FIND(hh, ctxDispMap, &pattern.key, sizeof(struct Key), element);
    HASH_DEL(ctxDispMap, element);
    pthread_rwlock_unlock(&ctxDispMapLock);
}

static GVDISdispatcherptr
get(EGLDisplay display, EGLContext context)
{
    struct Element *element;
    struct Element  pattern;

    memset(&pattern, 0, sizeof(struct Element));
    pattern.key.display = display;
    pattern.key.context = context;

    pthread_rwlock_rdlock(&ctxDispMapLock);
    HASH_FIND(hh, ctxDispMap, &pattern.key, sizeof(struct Key), element);
    pthread_rwlock_unlock(&ctxDispMapLock);

    return (element != NULL) ? element->dispatcher : NULL;
}

static int
put(EGLDisplay display, EGLContext context, GVDISdispatcherptr dispatcher)
{
    struct Element *newElement;

    if ((newElement = malloc(sizeof(struct Element))) == NULL)
    {
	perror("malloc");
	return -1;
    }	

    memset(newElement, 0, sizeof(struct Element));
    newElement->key.display      = display;
    newElement->key.context      = context;
    newElement->dispatcher        = dispatcher;
    newElement->markedForDeletion = 0;

    pthread_rwlock_wrlock(&ctxDispMapLock);
    HASH_ADD(hh, ctxDispMap, key, sizeof(struct Key), newElement);
    pthread_rwlock_unlock(&ctxDispMapLock);

    return 0;
}

/* ***************************************************************************
 * Process / thread initializers
 */

#define defaultDisplay (void *)0
#define defaultContext (void *)0

static int             processInitiated = 0;
static pthread_mutex_t initProcessLock  = PTHREAD_MUTEX_INITIALIZER;

static int
initProcess()
{
    int    vmShmFd   = atoi(environ[0]);
    size_t vmShmSize = atoi(environ[1]);

    GVTRPtransportptr  defaultTransport;
    GVDISdispatcherptr defaultDispatcher;
    size_t             offset;
    size_t             length;

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
	return -1;
    }

    if (put(defaultDisplay, defaultContext, defaultDispatcher) == -1)
    {
	return -1;
    }

    processInitiated = 1;

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
	        if (initProcess() == -1)			\
	        {						\
	            pthread_mutex_unlock(&initProcessLock);	\
	            /* return -1; */				\
		    exit(2);					\
	        }						\
            }							\
            pthread_mutex_unlock(&initProcessLock);		\
        }							\
    } while (0)

#define initThreadIfNotDoneAlready()					     \
    do {								     \
	if (gvdisGetCurrent() == NULL)					     \
	{								     \
	    if (gvdisMakeCurrent(get(defaultDisplay, defaultContext)) == -1) \
	    {								     \
		/* return -1; */					     \
		exit(2);						     \
	    }								     \
	}								     \
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
    GVSERcallid callId;
    EGLint      error;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

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

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

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

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_INITIALIZE);
    gvserInData(&display, sizeof(EGLDisplay));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserOutData(major,   sizeof(EGLint));
    gvserOutData(minor,   sizeof(EGLint));
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

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

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
    size_t       queryStringLength;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_QUERYSTRING);
    gvserInData(&display, sizeof(EGLDisplay));
    gvserInData(&name,    sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    /* Read string length: strlen(queryString) + 1 */
    gvserOutData(&queryStringLength, sizeof(size_t));
    /* Read actual string */
    queryString = malloc(queryStringLength * sizeof(char));
    gvserOutData(queryString, queryStringLength);
    gvserEndReturn();

    return queryString;
}

EGLBoolean
eglGetConfigs(EGLDisplay display, EGLConfig *configs, EGLint configSize, EGLint *numConfig)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_GETCONFIGS);
    gvserInData(&display,    sizeof(EGLDisplay));
    gvserInData(&configSize, sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status,   sizeof(EGLBoolean));
    gvserOutData(numConfig, sizeof(EGLint));
    /* NOTE configs has to be preallocated. If configs == NULL then no configs
     * are returned, only numConfig is returned (see spec. p. 23)
     */
    if (configs != NULL)
    {
	gvserOutData(configs, *numConfig * sizeof(EGLConfig));
    }
    gvserEndReturn();

    return status;
}

EGLBoolean
eglChooseConfig(EGLDisplay display, const EGLint *attribList, EGLConfig *configs, EGLint configSize, EGLint *numConfig)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    EGLint attribListSize;
    getAttribListSize(attribList, attribListSize);

    callId = gvserCall(GVSER_EGL_CHOOSECONFIG);
    gvserInData(&display,    sizeof(EGLDisplay));
    gvserInData(attribList,  attribListSize * sizeof(EGLint));
    gvserInData(&configSize, sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status,   sizeof(EGLBoolean));
    gvserOutData(numConfig, sizeof(EGLint));
    /* NOTE configs has to be preallocated, configs == NULL undefined ... */
    if (configs != NULL)
    {
	gvserOutData(configs, *numConfig * sizeof(EGLConfig));
    }
    gvserEndReturn();

    return status;
}

EGLBoolean
eglGetConfigAttrib(EGLDisplay display, EGLConfig config, EGLint attribute, EGLint *value)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_GETCONFIGATTRIB);    
    gvserInData(&display,   sizeof(EGLDisplay));
    gvserInData(&config,    sizeof(EGLConfig));
    gvserInData(&attribute, sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserOutData(value,   sizeof(EGLint));
    gvserEndReturn();

    return status;
}

EGLSurface
eglCreateWindowSurface(EGLDisplay display, EGLConfig config, EGLNativeWindowType window, const EGLint *attribList)
{
    GVSERcallid callId;
    EGLSurface  surface;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    EGLint attribListSize;
    getAttribListSize(attribList, attribListSize);

    callId = gvserCall(GVSER_EGL_CREATEWINDOWSURFACE);    
    gvserInData(&display,   sizeof(EGLDisplay));
    gvserInData(&config,    sizeof(EGLConfig));
    gvserInData(&window,    sizeof(EGLNativeWindowType));
    gvserInData(attribList, attribListSize * sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&surface, sizeof(EGLSurface));
    gvserEndReturn();

    return surface;
}

EGLSurface
eglCreatePbufferSurface(EGLDisplay display, EGLConfig config, const EGLint *attribList)
{
    GVSERcallid callId;
    EGLSurface  surface;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    EGLint attribListSize;
    getAttribListSize(attribList, attribListSize);

    callId = gvserCall(GVSER_EGL_CREATEPBUFFERSURFACE);    
    gvserInData(&display,   sizeof(EGLDisplay));
    gvserInData(&config,    sizeof(EGLConfig));
    gvserInData(attribList, attribListSize * sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&surface, sizeof(EGLSurface));
    gvserEndReturn();

    return surface;
}

EGLSurface
eglCreatePixmapSurface(EGLDisplay display, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attribList)
{
    GVSERcallid callId;
    EGLSurface  surface;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    EGLint attribListSize;
    getAttribListSize(attribList, attribListSize);

    callId = gvserCall(GVSER_EGL_CREATEPIXMAPSURFACE);    
    gvserInData(&display,   sizeof(EGLDisplay));
    gvserInData(&config,    sizeof(EGLConfig));
    gvserInData(&pixmap,    sizeof(EGLNativePixmapType));
    gvserInData(attribList, attribListSize * sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&surface, sizeof(EGLSurface));
    gvserEndReturn();

    return surface;
}

EGLBoolean
eglDestroySurface(EGLDisplay display, EGLSurface surface)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_DESTROYSURFACE);    
    gvserInData(&display, sizeof(EGLDisplay));
    gvserInData(&surface, sizeof(EGLSurface));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserEndReturn();

    return status;
}

EGLBoolean 
eglQuerySurface(EGLDisplay display, EGLSurface surface, EGLint attribute, EGLint *value)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_QUERYSURFACE);    
    gvserInData(&display,   sizeof(EGLDisplay));
    gvserInData(&surface,   sizeof(EGLSurface));
    gvserInData(&attribute, sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserOutData(value,   sizeof(EGLint));
    gvserEndReturn();

    return status;
}

EGLBoolean
eglBindAPI(EGLenum api)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_BINDAPI);    
    gvserInData(&api, sizeof(EGLBoolean));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserEndReturn();

    return status;
}

EGLenum
eglQueryAPI()
{
    GVSERcallid callId;
    EGLenum     api;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_QUERYAPI);    
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&api, sizeof(EGLenum));
    gvserEndReturn();

    return api;
}

EGLBoolean
eglWaitClient(void)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_WAITCLIENT);    
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserEndReturn();

    return status;
}

EGLBoolean
eglReleaseThread(void)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_RELEASETHREAD);    
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserEndReturn();

    return status;
}

EGLSurface
eglCreatePbufferFromClientBuffer(EGLDisplay display, EGLenum bufferType, EGLClientBuffer buffer, EGLConfig config, const EGLint *attribList)
{
    GVSERcallid callId;
    EGLSurface  surface;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    EGLint attribListSize;
    getAttribListSize(attribList, attribListSize);

    callId = gvserCall(GVSER_EGL_CREATEPBUFFERFROMCLIENTBUFFER);    
    gvserInData(&display,    sizeof(EGLDisplay));
    gvserInData(&bufferType, sizeof(EGLenum));
    gvserInData(&buffer,     sizeof(EGLClientBuffer));
    gvserInData(&config,     sizeof(EGLConfig));
    gvserInData(&attribList, attribListSize * sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&surface, sizeof(EGLSurface));
    gvserEndReturn();

    return surface;
}

EGLBoolean
eglSurfaceAttrib(EGLDisplay display, EGLSurface surface, EGLint attribute, EGLint value)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_SURFACEATTRIB);
    gvserInData(&display,   sizeof(EGLDisplay));
    gvserInData(&surface,   sizeof(EGLSurface));
    gvserInData(&attribute, sizeof(EGLint));
    gvserInData(&value,     sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserEndReturn();

    return status;
}

EGLBoolean
eglBindTexImage(EGLDisplay display, EGLSurface surface, EGLint buffer)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_BINDTEXIMAGE);
    gvserInData(&display, sizeof(EGLDisplay));
    gvserInData(&surface, sizeof(EGLSurface));
    gvserInData(&buffer,  sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserEndReturn();

    return status;
}

EGLBoolean
eglReleaseTexImage(EGLDisplay display, EGLSurface surface, EGLint buffer)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_RELEASETEXIMAGE);
    gvserInData(&display, sizeof(EGLDisplay));
    gvserInData(&surface, sizeof(EGLSurface));
    gvserInData(&buffer,  sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserEndReturn();

    return status;
}

EGLBoolean
eglSwapInterval(EGLDisplay display, EGLint interval)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_SWAPINTERVAL);
    gvserInData(&display,  sizeof(EGLDisplay));
    gvserInData(&interval, sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserEndReturn();

    return status;
}

EGLContext
eglCreateContext(EGLDisplay display, EGLConfig config, EGLContext shareContext, const EGLint *attribList)
{
    GVSERcallid callId;
    EGLContext  context;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    EGLint attribListSize;
    getAttribListSize(attribList, attribListSize);

    callId = gvserCall(GVSER_EGL_CREATECONTEXT);
    gvserInData(&display,      sizeof(EGLDisplay));
    gvserInData(&config,       sizeof(EGLConfig));
    gvserInData(&shareContext, sizeof(EGLContext));
    gvserInData(&attribList,   attribListSize * sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&context, sizeof(EGLContext));
    gvserEndReturn();

    return context;
}

EGLBoolean
eglDestroyContext(EGLDisplay display, EGLContext context)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_DESTROYCONTEXT);
    gvserInData(&display, sizeof(EGLDisplay));
    gvserInData(&context, sizeof(EGLContext));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserEndReturn();

    return status;
}

EGLBoolean
eglMakeCurrent(EGLDisplay display, EGLSurface drawSurface, EGLSurface readSurface, EGLContext context)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_MAKECURRENT);
    gvserInData(&display,     sizeof(EGLDisplay));
    gvserInData(&drawSurface, sizeof(EGLSurface));
    gvserInData(&readSurface, sizeof(EGLSurface));
    gvserInData(&context,     sizeof(EGLContext));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserEndReturn();

    return status;
}

EGLContext eglGetCurrentContext()
{
    GVSERcallid callId;
    EGLContext  context;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_GETCURRENTCONTEXT);
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&context, sizeof(EGLContext));
    gvserEndReturn();

    return context;
}

EGLSurface eglGetCurrentSurface(EGLint readdraw)
{
    GVSERcallid callId;
    EGLSurface  surface;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_GETCURRENTSURFACE);
    gvserInData(&readdraw, sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&surface, sizeof(EGLSurface));
    gvserEndReturn();

    return surface;
}

EGLDisplay eglGetCurrentDisplay()
{
    GVSERcallid callId;
    EGLDisplay  display;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_GETCURRENTDISPLAY);
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&display, sizeof(EGLDisplay));
    gvserEndReturn();

    return display;
}

EGLBoolean eglQueryContext(EGLDisplay display, EGLContext context, EGLint attribute, EGLint *value)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_QUERYCONTEXT);
    gvserInData(&display,   sizeof(EGLDisplay));
    gvserInData(&context,   sizeof(EGLContext));
    gvserInData(&attribute, sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserOutData(value,   sizeof(EGLint));
    gvserEndReturn();

    return status;
}

EGLBoolean
eglWaitGL()
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_WAITGL);
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserEndReturn();

    return status;
}

EGLBoolean
eglWaitNative(EGLint engine)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_WAITNATIVE);
    gvserInData(&engine, sizeof(EGLint));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserEndReturn();

    return status;
}

EGLBoolean
eglSwapBuffers(EGLDisplay display, EGLSurface surface)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_SWAPBUFFERS);
    gvserInData(&display, sizeof(EGLDisplay));
    gvserInData(&surface, sizeof(EGLSurface));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserEndReturn();

    return status;
}

EGLBoolean
eglCopyBuffers(EGLDisplay display, EGLSurface surface, EGLNativePixmapType target)
{
    GVSERcallid callId;
    EGLBoolean  status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_COPYBUFFERS);
    gvserInData(&display, sizeof(EGLDisplay));
    gvserInData(&surface, sizeof(EGLSurface));
    gvserInData(&target,  sizeof(EGLNativePixmapType));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserEndReturn();

    return status;
}
