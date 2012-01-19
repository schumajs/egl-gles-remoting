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
    int                markCurrent;
    int                markDeleted;

    UT_hash_handle     hh;
};

static struct Element    *ctxDispMap     = NULL;
static pthread_rwlock_t   ctxDispMapLock = PTHREAD_RWLOCK_INITIALIZER;

static void
del(struct Element *element)
{
    pthread_rwlock_wrlock(&ctxDispMapLock);
    HASH_DEL(ctxDispMap, element);
    pthread_rwlock_unlock(&ctxDispMapLock);
}

static struct Element
*get(EGLDisplay display, EGLContext context)
{
    struct Element *element;
    struct Element  pattern;

    memset(&pattern, 0, sizeof(struct Element));
    pattern.key.display = display;
    pattern.key.context = context;

    pthread_rwlock_rdlock(&ctxDispMapLock);
    HASH_FIND(hh, ctxDispMap, &pattern.key, sizeof(struct Key), element);
    pthread_rwlock_unlock(&ctxDispMapLock);

    return element;
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
    newElement->key.display = display;
    newElement->key.context = context;
    newElement->dispatcher  = dispatcher;
    newElement->markCurrent = 0;
    newElement->markDeleted = 0;

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

static GVSHMshm vmShm;
static size_t   vmShmSize;

static int             processInitiated = 0;
static pthread_mutex_t initProcessLock  = PTHREAD_MUTEX_INITIALIZER;

static int
initProcess()
{
    GVTRPtransportptr  defaultTransport;
    GVDISdispatcherptr defaultDispatcher;
    size_t             offset;
    size_t             length;

    vmShm     = atoi(environ[0]); 
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

    if (gvsrvBonjour(offset, length) == -1)
    {
	perror("gvsrvHandshake");
	return -1;
    }

    if (gvtrpCreate(&defaultTransport, &vmShm, offset, length) == -1)
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
	perror("put");
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
	            /* TODO ? */				\
		    exit(2);					\
	        }						\
            }							\
            pthread_mutex_unlock(&initProcessLock);		\
        }							\
    } while (0)

#define initThreadIfNotDoneAlready()					 \
    do {								 \
	if (gvdisGetCurrent() == NULL)					 \
	{								 \
            struct Element *element;					 \
									 \
	    if ((element = get(defaultDisplay, defaultContext)) == NULL) \
	    {								 \
		/* TODO ? */						 \
                exit(2);						 \
	    }								 \
									 \
	    if (gvdisMakeCurrent(element->dispatcher) == -1)		 \
	    {								 \
		/* TODO ? */						 \
		exit(2);						 \
	    }								 \
	}								 \
    } while (0)

/* ***************************************************************************
 * Client / Server Coordination
 */

static int
notifyCreateContext(size_t offset, size_t length)
{
    GVSERcallid callId;
    int         status;

    callId = gvserCall(GVSER_RESERVED_0);
    gvserInData(&offset, sizeof(size_t));
    gvserInData(&length, sizeof(size_t));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(int));
    gvserEndReturn();

    return status;
}

static int
createContext(EGLDisplay display, EGLContext context)
{
    GVTRPtransportptr  transport;
    GVDISdispatcherptr dispatcher;
    GVDISdispatcherptr tempDispatcher;
    size_t             offset;
    size_t             length;

    /* Cache the current dispatcher since gvmmgrAlloc switches to its own
     * dispatcher (different shared memory).
     */
    if ((tempDispatcher = gvdisGetCurrent()) == NULL)
    {
	perror("gvdisGetCurrent");
	return -1;
    }

    /* Allocate heap memory; used by the new transport */

    length = 101 * 4096;

    if (gvmmgrAlloc(&offset, length) == -1)
    {
	perror("gvmmgrAlloc");
	return -1;
    }

    /* Create transport */
    if (gvtrpCreate(&transport, &vmShm, offset, length) == -1)
    {
	perror("gvtrpCreate");
	return -1;
    }

    /* Create dispatcher */
    if (gvdisCreate(&dispatcher, transport) == -1)
    {
	perror("gvdisCreate");
	return -1;
    }    

    /* Switch back to the original dispatcher.  */
    if (gvdisMakeCurrent(tempDispatcher) == -1)
    {
	perror("gvdisMakeCurrent");
	return -1;
    }

    /* Let the server know that a new server dispatcher should be created  */
    if (notifyCreateContext(offset, length) == -1)
    {
	perror("notifyDestroyContext");
	return -1;
    }

    /* Remember the new EGL context and its dispatcher */
    if (put(display, context, dispatcher) == -1)
    {
	perror("put");
	return -1;
    }

    return 0;
}

static int
notifyDestroyContext()
{
    GVSERcallid callId;    
    int         status;

    callId = gvserCall(GVSER_RESERVED_1);
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(int));
    gvserEndReturn();

    return status;
}

static int
destroyContext(struct Element *element)
{
    GVDISdispatcherptr tempDispatcher;
    size_t             tempTransportOffset;		 
   
    tempTransportOffset = element->dispatcher->transport->offset;

    if ((tempDispatcher = gvdisGetCurrent()) == NULL)
    {
	perror("gvdisGetCurrent");
	return -1;
    }

    if (tempDispatcher != element->dispatcher)
    {
	/* Switch to the to-be destroyed dispatcher */
	if (gvdisMakeCurrent(element->dispatcher) == -1)
	{
	    perror("gvdisMakeCurrent");
	    return -1;
	}
    }
    else
    {
	/* Thread has to be reinitialized if we destroy the thread's current
         * dispatcher. So set it to NULL (see initThreadIfNotDoneAlready()).
         */
	tempDispatcher = NULL;
    }

    /* Let the server know that the corresponding server dispatcher is not
     * needed anymore.
     */
    if (notifyDestroyContext() == -1)
    {
	perror("notifyDestroyContext");
	return -1;
    }

    /* Destroy transport */
    if (gvtrpDestroy(element->dispatcher->transport) == -1)
    {
	perror("gvtrpDestroy");
	return -1;
    }

    /* Destroy dispatcher */
    if (gvdisDestroy(element->dispatcher) == -1)
    {
	perror("gvdisDestroy");
	return -1;
    }

    /* Free the shared heap space used by that transport */
    if (gvmmgrFree(tempTransportOffset) == -1)
    {
	perror("gvmmgrFree");
	return -1;
    }

    /* Switch back to the original dispatcher, NULL if we deleted the thread's
     * current dispatcher
     */
    if (gvdisMakeCurrent(tempDispatcher) == -1)
    {
	perror("gvdisMakeCurrent");
	return -1;
    }

    /* Forget about the EGL context and its dispatcher */
    del(element);

    return 0;
}

#define destroyOrMarkForDeletion(element)		\
    do {						\
	if (element->markCurrent)			\
	{						\
	    /* Context is current to any thread */	\
	    element->markDeleted = 1;			\
	}						\
	else						\
	{						\
	    /* Context is NOT current to any thread */	\
	    if (destroyContext(element) == -1)		\
	    {						\
		perror("destroyContext");		\
		exit(2);				\
	    }						\
	}						\
    } while (0);

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
 * TODO gvsrvAuRevoir() if last display is terminated. What to do in case of
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

    /* Destroy / mark for deletion all the dispatchers that belong to the
     * display.
     */
    if (status == EGL_SUCCESS)
    {
	struct Element *element, *tempElement;

	HASH_ITER(hh, ctxDispMap, element, tempElement) {
	    if (element->key.display == display)
	    {
		destroyOrMarkForDeletion(element);
	    }
	}
    }

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

    EGLDisplay tempDisplay;
    EGLContext tempContext;

    /* TODO cache display and context to avoid RPC */
    tempDisplay = eglGetCurrentDisplay();
    tempContext = eglGetCurrentContext();

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    callId = gvserCall(GVSER_EGL_RELEASETHREAD);    
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserEndReturn();

    /* Destroy transport */
    if (status == EGL_SUCCESS)
    {
	struct Element *element = get(tempDisplay, tempContext);

	if (element->markDeleted)
	{
	    /* The thread's current context has been marked for deletion */
	    if (destroyContext(element) == -1)
	    {
		perror("destroyContext");
		exit(2);
	    }
	}
    }

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

    if (context != EGL_NO_CONTEXT)
    {

    }

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
    gvserInData(&display ,sizeof(EGLDisplay));
    gvserInData(&context, sizeof(EGLContext));
    gvserEndCall();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserEndReturn();

    if (status == EGL_SUCCESS)
    {
	destroyOrMarkForDeletion(get(display, context));
    }

    return status;
}

EGLBoolean
eglMakeCurrent(EGLDisplay display, EGLSurface drawSurface, EGLSurface readSurface, EGLContext context)
{
    GVSERcallid callId;
    EGLBoolean  status;

    EGLDisplay tempDisplay;
    EGLContext tempContext;

    /* TODO cache display and context to avoid RPC */
    tempDisplay = eglGetCurrentDisplay();
    tempContext = eglGetCurrentContext();

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

    if (status == EGL_SUCCESS)
    {
	struct Element *element, *tempElement;

	/* !!!
	 * TODO make thread-safe
         * !!!
         */
	element = get(display, context);

	element->markCurrent = 1;

	if(gvdisMakeCurrent(element->dispatcher) == -1)
	{
	    perror("gvdisMakeCurrent");
	    exit(2);
	}

	tempElement = get(tempDisplay, tempContext);

	if (tempElement->markDeleted)
	{
	    destroyContext(tempElement);
	}
	else
	{
	    tempElement->markCurrent = 0;
	}
    }

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
