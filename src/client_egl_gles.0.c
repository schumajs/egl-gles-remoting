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
#include <string.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "client_serializer.h"
#include "client_state_tracker.0.h"
#include "error.h"
#include "heap_manager.h"
#include "janitor.h"

#define TRANSPORT_LENGTH 11 * 4096

#define DEFAULT_DISPLAY  (void *)0
#define DEFAULT_CONTEXT  (void *)0

extern char            **environ;

static GVshmptr          vmShm;

static int               processInitiated = 0;
static pthread_mutex_t   initProcessLock  = PTHREAD_MUTEX_INITIALIZER;

static int
createContext(EGLDisplay display, EGLContext context)
{
    GVcontextstateptr contextState;
    GVtransportptr    transport;
    size_t            transportOffset;

    TRY
    {
	if ((contextState = malloc(sizeof(struct GVcontextstate))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	if ((transportOffset = gvAlloc(TRANSPORT_LENGTH)) == 0)
	{
	    THROW(e0, "gvAlloc");
	}

	if (gvBonjour(transportOffset, TRANSPORT_LENGTH) == -1)
	{
	    THROW(e0, "gvBonjour");
	}

	if ((transport = gvCreateTransport(vmShm,
					   transportOffset,
					   TRANSPORT_LENGTH)) == NULL)
	{
	    THROW(e0, "gvCreateTransport");
	}

	contextState->display         = display;
	contextState->context         = context;
	contextState->markedCurrent   = 0;
	contextState->markedDestroyed = 0;
	contextState->transport       = transport;

	if (gvPutEglContextState(display, context, contextState) == -1)
	{
	    THROW(e0, "gvPutEglContextState");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

static int
makeCurrent(EGLDisplay newDisplay, EGLContext newContext)
{
    GVcontextstateptr newContextState;
    EGLDisplay        oldDisplay;
    EGLContext        oldContext;

    TRY
    {
	if ((oldDisplay = gvGetCurrentThreadDisplay()) == NULL)
	{
	    THROW(e0, "gvGetCurrentThreadDisplay");
	}

	if ((oldContext = gvGetCurrentThreadContext()) == NULL)
	{
	    THROW(e0, "gvGetCurrentThreadContext");
	}

	if (gvSetMarkedCurrent(oldDisplay, oldContext, 0) == -1)
	{
	    THROW(e0, "gvSetMarkedCurrent");
	}

	if ((newContextState
	     = gvGetEglContextState(newDisplay, newContext)) == NULL)
	{
	    THROW(e0, "gvGetEglContextState");
	}

	if (gvSetCurrentThreadDisplay(newContextState->display) == -1)
	{
	    THROW(e0, "gvSetThreadDisplay");
	}

	if (gvSetCurrentThreadContext(newContextState->context) == -1)
	{
	    THROW(e0, "gvSetThreadContext");
	}

	if (gvSetCurrentThreadTransport(newContextState->transport) == -1)
	{
	    THROW(e0, "gvSetThreadTransport");
	}

	if (gvSetMarkedCurrent(newDisplay, newContext, 1) == -1)
	{
	    THROW(e0, "gvSetMarkedCurrent");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

static int
initEglGlesClient()
{
    int vmShmFd    = atoi(environ[0]);
    int vmShmSize  = atoi(environ[1]);

    TRY
    {
	if ((vmShm = malloc(sizeof(struct GVshm))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	vmShm->id   = vmShmFd;
	vmShm->size = vmShmSize;

	if (vmShmSize < TRANSPORT_LENGTH)
	{
	    errno = EINVAL;
	    THROW(e0, "not enough space");
	}

	if (createContext(DEFAULT_DISPLAY, DEFAULT_CONTEXT) == -1)
	{
	    THROW(e0, "createContext");
	}

	if (gvSetCurrentThreadDisplay(DEFAULT_DISPLAY) == -1)
	{
	    THROW(e0, "gvSetCurrentThreadDisplay");
	}

	if (gvSetCurrentThreadContext(DEFAULT_CONTEXT) == -1)
	{
	    THROW(e0, "gvSetCurrentThreadContext");
	}

	if (gvSetMarkedCurrent(DEFAULT_CONTEXT, DEFAULT_DISPLAY, 1) == -1)
	{
	    THROW(e0, "gvSetMarkedCurrent");
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

static int
getAttribListSize(const EGLint *attribList)
{
    int attribListSize = 0;

    if (attribList != NULL)
    {
	while (attribList[attribListSize] != EGL_NONE)
	{
	    attribListSize++;
	}
    }
    
    return attribListSize;
}

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
    char           *queryString;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    callId = gvStartSending(transport, NULL, GV_CMDID_EGL_QUERYSTRING);
    gvSendData(transport, &display, sizeof(EGLDisplay));
    gvSendData(transport, &name, sizeof(EGLint));

    gvStartReceiving(transport, NULL, callId);
    queryString = gvReceiveVarSizeData(transport);

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

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    /* A - Optional OUT pointer */
    if (configs == NULL)
    {
	configSize = 0;
    }

    callId = gvStartSending(transport, NULL, GV_CMDID_EGL_GETCONFIGS);
    gvSendData(transport, &display, sizeof(EGLDisplay));
    gvSendData(transport, &configSize, sizeof(EGLint));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, &status, sizeof(EGLBoolean));
    gvReceiveData(transport, numConfig, sizeof(EGLint));

    /* B - Optional OUT pointer */
    if (configs != NULL)
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

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    /* A - Optional OUT pointer */
    if (configs == NULL)
    {
	configSize = 0;
    }

    callId = gvStartSending(transport, NULL, GV_CMDID_EGL_CHOOSECONFIG);
    gvSendData(transport, &display, sizeof(EGLDisplay));
    gvSendVarSizeData(transport,
		      attribList,
		      getAttribListSize(attribList) * sizeof(EGLint));
    gvSendData(transport, &configSize, sizeof(EGLint));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, &status, sizeof(EGLBoolean));
    gvReceiveData(transport, numConfig, sizeof(EGLint));

    /* B - Optional OUT pointer */
    if (configs != NULL)
    {
        gvReceiveData(transport, configs, *numConfig * sizeof(EGLConfig));
    }

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

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    callId = gvStartSending(transport, NULL, GV_CMDID_EGL_CREATEWINDOWSURFACE);
    gvSendData(transport, &display, sizeof(EGLDisplay));
    gvSendData(transport, &config, sizeof(EGLConfig));
    gvSendData(transport, &window, sizeof(EGLNativeWindowType));
    gvSendVarSizeData(transport,
		      attribList,
		      getAttribListSize(attribList) * sizeof(EGLint));

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
    GVtransportptr  transport;

    GVcallid        callId;
    EGLContext      context;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    callId = gvStartSending(transport, NULL, GV_CMDID_EGL_CREATECONTEXT);
    gvSendData(transport, &display, sizeof(EGLDisplay));
    gvSendData(transport, &config, sizeof(EGLConfig));
    gvSendData(transport, &shareContext, sizeof(EGLContext));
    gvSendVarSizeData(transport,
		      attribList,
		      getAttribListSize(attribList) * sizeof(EGLint));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, &context, sizeof(EGLContext));

    if (context != NULL)
    {
	TRY
	{
	    if (createContext(display, context) == -1)
	    {
		THROW(e0, "createContext");
	    }
	}
	CATCH (e0)
	{
	    return NULL;
	}
    }

    return context;
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
    GVtransportptr  transport;

    GVcallid        callId;
    EGLBoolean      status;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    callId = gvStartSending(transport, NULL, GV_CMDID_EGL_MAKECURRENT);
    gvSendData(transport, &display, sizeof(EGLDisplay));
    gvSendData(transport, &drawSurface, sizeof(EGLSurface));
    gvSendData(transport, &readSurface, sizeof(EGLSurface));
    gvSendData(transport, &context, sizeof(EGLContext));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, &status, sizeof(EGLBoolean));

    if (status == EGL_TRUE)
    {
	TRY
	{
	    if (makeCurrent(display, context) == -1)
	    {
		THROW(e0, "makeCurrent");
	    }
	}
	CATCH (e0)
	{
	    return EGL_FALSE;
	}
    }

    return EGL_TRUE;
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

/* ***************************************************************************
 * GLES2
 */

GLuint
glCreateShader(GLenum type)
{
    GVtransportptr  transport;

    GVcallid        callId;
    GLuint          shaderId;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    callId = gvStartSending(transport, NULL, GV_CMDID_GLES2_CREATESHADER);
    gvSendData(transport, &type, sizeof(GLenum));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, &shaderId, sizeof(GLuint));

    return shaderId;
}

void
glShaderSource(GLuint         shader,
	       GLsizei        count,
	       const GLchar **string,
	       const GLint*   length)
{
    GVtransportptr  transport;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    gvStartSending(transport, NULL, GV_CMDID_GLES2_SHADERSOURCE);
    gvSendData(transport, &shader, sizeof(GLuint));
    gvSendData(transport, &count, sizeof(GLsizei));

    /* There are several possibilities to pass "string", see spec. p. 27 */
    if (length == NULL)
    {
	int passingType = 0;
	gvSendData(transport, &passingType, sizeof(int));

	int i;
	for (i = 0; i < count; i++)
	{
	    gvSendVarSizeData(transport,
			      *string,
			      (strlen(string[i]) + 1) * sizeof(char));
	}
    }
    else
    {
	int passingType = 1;
	gvSendData(transport, &passingType, sizeof(int));

	gvSendVarSizeData(transport, length, count * sizeof(GLint));

	int i;
	for (i = 0; i < count; i++)
	{
	    if (length[i] < 0)
	    {
		gvSendVarSizeData(transport,
				  *string,
				  (strlen(string[i]) + 1) * sizeof(char));
	    }
	    else
	    {
		gvSendVarSizeData(transport,
				  *string,
				  (length[i] + 1) * sizeof(char));
	    }
	}
    }
}

void
glCompileShader(GLuint shader)
{
    GVtransportptr  transport;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    gvStartSending(transport, NULL, GV_CMDID_GLES2_COMPILESHADER);
    gvSendData(transport, &shader, sizeof(GLuint));
}

void
glGetShaderiv(GLuint  shader,
	      GLenum  pname,
	      GLint  *params)
{
    GVtransportptr  transport;

    GVcallid        callId;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    callId = gvStartSending(transport, NULL, GV_CMDID_GLES2_GETSHADERIV);
    gvSendData(transport, &shader, sizeof(GLuint));
    gvSendData(transport, &pname, sizeof(GLenum));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, params, sizeof(GLint));
}

void
glGetShaderInfoLog(GLuint   shader,
		   GLsizei  bufsize,
		   GLsizei *length,
		   GLchar  *infolog)
{
    GVtransportptr  transport;

    GVcallid        callId;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    callId = gvStartSending(transport, NULL, GV_CMDID_GLES2_GETSHADERINFOLOG);
    gvSendData(transport, &shader, sizeof(GLuint));
    gvSendData(transport, &bufsize, sizeof(GLsizei));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, length, sizeof(GLsizei));
    gvReceiveData(transport, infolog, *length * sizeof(GLchar));
}

void
glDeleteShader(GLuint shader)
{
    GVtransportptr  transport;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    gvStartSending(transport, NULL, GV_CMDID_GLES2_DELETESHADER);
    gvSendData(transport, &shader, sizeof(GLuint));
}

GLuint
glCreateProgram()
{
    GVtransportptr  transport;

    GVcallid        callId;
    GLuint          program;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    callId = gvStartSending(transport, NULL, GV_CMDID_GLES2_CREATEPROGRAM);

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, &program, sizeof(GLuint));

    return program;
}

void
glAttachShader(GLuint program,
	       GLuint shader)
{
    GVtransportptr  transport;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    gvStartSending(transport, NULL, GV_CMDID_GLES2_ATTACHSHADER);
    gvSendData(transport, &program, sizeof(GLuint));
    gvSendData(transport, &shader, sizeof(GLuint));
}

void
glBindAttribLocation(GLuint        program,
		     GLuint        index,
		     const GLchar *name)
{
    GVtransportptr  transport;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    gvStartSending(transport, NULL, GV_CMDID_GLES2_BINDATTRIBLOCATION);
    gvSendData(transport, &program, sizeof(GLuint));
    gvSendData(transport, &index, sizeof(GLuint));
    gvSendVarSizeData(transport, name, (strlen(name) + 1) * sizeof(GLchar));
}

void
glLinkProgram(GLuint program)
{
    GVtransportptr  transport;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    gvStartSending(transport, NULL, GV_CMDID_GLES2_LINKPROGRAM);
    gvSendData(transport, &program, sizeof(GLuint));
}

void
glGetProgramiv(GLuint  program,
	       GLenum  pname,
	       GLint  *params)
{
    GVtransportptr  transport;

    GVcallid        callId;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    callId = gvStartSending(transport, NULL, GV_CMDID_GLES2_GETPROGRAMIV);
    gvSendData(transport, &program, sizeof(GLuint));
    gvSendData(transport, &pname, sizeof(GLenum));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, params, sizeof(GLint));
}

void
glGetProgramInfoLog(GLuint   program,
		    GLsizei  bufsize,
		    GLsizei *length,
		    GLchar  *infolog)
{
    GVtransportptr  transport;

    GVcallid        callId;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    callId = gvStartSending(transport, NULL, GV_CMDID_GLES2_GETPROGRAMINFOLOG);
    gvSendData(transport, &program, sizeof(GLuint));
    gvSendData(transport, &bufsize, sizeof(GLsizei));

    gvStartReceiving(transport, NULL, callId);
    gvReceiveData(transport, length, sizeof(GLsizei));
    gvReceiveData(transport, infolog, *length * sizeof(GLchar));
}

void
glDeleteProgram(GLuint program)
{
    GVtransportptr  transport;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    gvStartSending(transport, NULL, GV_CMDID_GLES2_DELETEPROGRAM);
    gvSendData(transport, &program, sizeof(GLuint));
}

void
glClearColor(GLclampf red,
	     GLclampf green,
	     GLclampf blue,
	     GLclampf alpha)
{
    GVtransportptr  transport;

    initProcessIfNotDoneAlready();
    initThreadIfNotDoneAlready();

    transport = gvGetCurrentThreadTransport();

    gvStartSending(transport, NULL, GV_CMDID_GLES2_CLEARCOLOR);
    gvSendData(transport, &red, sizeof(GLclampf));
    gvSendData(transport, &green, sizeof(GLclampf));
    gvSendData(transport, &blue, sizeof(GLclampf));
    gvSendData(transport, &alpha, sizeof(GLclampf));
}
