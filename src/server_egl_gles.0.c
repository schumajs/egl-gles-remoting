/*! ***************************************************************************
 * \file    server_egl_gles.0.c
 * \brief   
 * 
 * \date    January 9, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <stdlib.h>
#include <string.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "error.h"
#include "server_dispatcher.h"
#include "server_serializer.h"
#include "server_state_tracker.0.h"

static
void _notInUse()
{

}

/* ***************************************************************************
 * EGL
 */

static
void _eglGetError()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    EGLint         error;

    gvReceiveData(transport, &callId, sizeof(GVcallid));

    error = eglGetError();

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &error, sizeof(EGLint));
}

static
void _eglGetDisplay()
{
    GVtransportptr       transport = gvGetCurrentThreadTransport();

    GVcallid             callId;
    EGLDisplay           display;
    EGLNativeDisplayType displayId;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &displayId, sizeof(EGLNativeDisplayType));

    display = eglGetDisplay(displayId);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &display, sizeof(EGLDisplay));
}

static
void _eglInitialize()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();
    
    GVcallid       callId;
    EGLBoolean     status;
    EGLDisplay     display;
    EGLint         minor;
    EGLint         major;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));

    status = eglInitialize(display, &minor, &major);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &status, sizeof(EGLBoolean));
    gvSendData(transport, &major, sizeof(EGLint));
    gvSendData(transport, &minor, sizeof(EGLint));
}

static
void _eglTerminate()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    EGLBoolean     status;
    EGLDisplay     display;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));

    status = eglTerminate(display);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &status, sizeof(EGLBoolean));
}

void
_eglQueryString()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    EGLDisplay      display;
    EGLint          name;
    const char     *queryString;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    gvReceiveData(transport, &name, sizeof(EGLint));

    queryString = eglQueryString(display, name);

    gvStartSending(transport, NULL, callId);
    gvSendVarSizeData(transport,
		      queryString,
		      (strlen(queryString) + 1) * sizeof(char));
}

void
_eglGetConfigs()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    EGLBoolean      status;
    EGLDisplay      display;
    EGLConfig      *configs;
    EGLint          configSize;
    EGLint          numConfig;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    gvReceiveData(transport, &configSize, sizeof(EGLint));
    
    /* A - optional OUT pointer */
    if (configSize > 0)
    {
	configs = malloc(configSize);
    }
    else
    {
	configs = NULL;
    }
    
    status = eglGetConfigs(display, configs, configSize, &numConfig);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &status, sizeof(EGLBoolean));    
    gvSendData(transport, &numConfig, sizeof(EGLint));

    /* B - optional OUT pointer */
    if (configSize > 0)
    {
	gvSendData(transport, configs, numConfig * sizeof(EGLConfig));	
	free(configs);
    }
}

void
_eglChooseConfig()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    EGLBoolean      status;
    EGLDisplay      display;
    EGLint         *attribList;
    EGLConfig      *configs;
    EGLint          configSize;
    EGLint          numConfig;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    attribList = (EGLint *)gvReceiveVarSizeData(transport);
    gvReceiveData(transport, &configSize, sizeof(EGLint));

    /* A - optional OUT pointer */
    if (configSize > 0)
    {
	configs = malloc(configSize);
    }
    else
    {
	configs = NULL;
    }
   
    configs = malloc(configSize * sizeof(EGLConfig));

    status = eglChooseConfig(display, attribList, configs, configSize, &numConfig);

    free(attribList);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &status, sizeof(EGLBoolean));
    gvSendData(transport, &numConfig, sizeof(EGLint));

    /* B - optional OUT pointer */
    if (configSize > 0)
    {
	gvSendData(transport, configs, numConfig * sizeof(EGLConfig));	
	free(configs);
    }
}

void
_eglGetConfigAttrib()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    EGLBoolean      status;
    EGLDisplay      display;
    EGLConfig       config;
    EGLint          attribute;
    EGLint          value;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    gvReceiveData(transport, &config, sizeof(EGLConfig));
    gvReceiveData(transport, &attribute, sizeof(EGLint));

    status = eglGetConfigAttrib(display, config, attribute, &value);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &status, sizeof(EGLBoolean));
    gvSendData(transport, &value, sizeof(EGLint));
}

void
_eglCreateWindowSurface()
{
    GVtransportptr       transport = gvGetCurrentThreadTransport();

    GVcallid             callId;
    EGLSurface           surface;
    EGLDisplay           display;
    EGLConfig            config;
    EGLNativeWindowType  window;
    EGLint              *attribList;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    gvReceiveData(transport, &config, sizeof(EGLConfig));
    gvReceiveData(transport, &window, sizeof(EGLNativeWindowType));
    attribList = (EGLint *)gvReceiveVarSizeData(transport);

    surface = eglCreateWindowSurface(display, config, window, attribList);

    free(attribList);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &surface, sizeof(EGLSurface));
}

void
_eglCreatePbufferSurface()
{

}

void
_eglCreatePixmapSurface()
{

}

void
_eglDestroySurface()
{

}

void
_eglQuerySurface()
{

}

void
_eglBindAPI()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    EGLBoolean      status;
    EGLenum         api;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &api, sizeof(EGLenum));

    status = eglBindAPI(api);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &status, sizeof(EGLBoolean));
}

void
_eglQueryAPI()
{

}

void
_eglWaitClient()
{

}

void
_eglReleaseThread()
{

}

void
_eglCreatePbufferFromClientBuffer()
{

}

void
_eglSurfaceAttrib()
{

}

void
_eglBindTexImage()
{

}

void
_eglReleaseTexImage()
{

}

void
_eglSwapInterval()
{

}

void
_eglCreateContext()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    EGLContext      context;
    EGLDisplay      display;
    EGLConfig       config;
    EGLContext      shareContext;
    EGLint         *attribList;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    gvReceiveData(transport, &config, sizeof(EGLConfig));
    gvReceiveData(transport, &shareContext, sizeof(EGLContext));
    attribList = (EGLint *)gvReceiveVarSizeData(transport);

    context = eglCreateContext(display, config, shareContext, attribList);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &context, sizeof(EGLContext));
}

void
_eglDestroyContext()
{

}

void
_eglMakeCurrent()
{

}

void
_eglGetCurrentContext()
{

}

void
_eglGetCurrentSurface()
{

}

void
_eglGetCurrentDisplay()
{

}

void
_eglQueryContext()
{

}

void
_eglWaitGL()
{

}

void
_eglWaitNative()
{

}

void
_eglSwapBuffers()
{

}

void
_eglCopyBuffers()
{

}

/* ***************************************************************************
 * GLES2
 */

void
_glCreateShader()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         shader;
    GLenum         type;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &type, sizeof(GLenum));

    shader = glCreateShader(type);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &shader, sizeof(GLuint));
}

void
_glShaderSource()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         shader;
    GLsizei        count;
    GLchar       **string;
    GLint         *length;
    
    int             passingType;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &shader, sizeof(GLuint));
    gvReceiveData(transport, &count, sizeof(GLsizei));

    string = malloc(count * sizeof(GLchar*));

    /* There are several possibilities to pass "string", see spec. p. 27 */
    gvReceiveData(transport, &passingType, sizeof(int));

    if (passingType == 0)
    {
	length = NULL;
    }
    else if (passingType == 1)
    {
	length = gvReceiveVarSizeData(transport);
    }

    int i;
    for (i = 0; i < count; i++)
    {
	string[i] = gvReceiveVarSizeData(transport);
    }

    glShaderSource(shader, count, string, length);

    if (passingType == 0)
    {

    }
    else if (passingType == 1)
    {
	free(length);
    }

    for (i = 0; i < count; i++)
    {
	free(string[i]);
    }

    free(string);
}

void
_glCompileShader()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         shader;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &shader, sizeof(GLuint));

    glCompileShader(shader);
}

void
_glGetShaderiv()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         shader;
    GLenum         pname;
    GLint          params;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &shader, sizeof(GLuint));
    gvReceiveData(transport, &pname, sizeof(GLenum));

    glGetShaderiv(shader, pname, &params);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &params, sizeof(GLint));
}

void
_glGetShaderInfoLog()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         shader;
    GLsizei        bufsize;
    GLsizei        length;
    GLchar        *infoLog;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &shader, sizeof(GLuint));
    gvReceiveData(transport, &bufsize, sizeof(GLsizei));

    infoLog = malloc(bufsize * sizeof(GLchar));

    glGetShaderInfoLog(shader, bufsize, &length, infoLog);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &length, sizeof(GLsizei));
    gvSendData(transport, infoLog, length * sizeof(GLchar));
}

void
_glDeleteShader()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         shader;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &shader, sizeof(GLuint));

    glDeleteShader(shader);
}

void
_glCreateProgram()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         program;

    gvReceiveData(transport, &callId, sizeof(GVcallid));

    program = glCreateProgram();

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &program, sizeof(GLuint));
}

/* ****************************************************************************
 * Jump table
 */

GVdispatchfunc eglGlesJumpTable[43] = {
    _notInUse,
    _notInUse,
    _notInUse,
    _notInUse,
    _notInUse,
    _notInUse,
    _notInUse,
    _notInUse,
    _notInUse,
    _notInUse,
    _eglGetError,
    _eglGetDisplay,
    _eglInitialize,
    _eglTerminate,
    _eglQueryString,
    _eglGetConfigs,
    _eglChooseConfig,
    _eglGetConfigAttrib,
    _eglCreateWindowSurface,
    _eglCreatePbufferSurface,
    _eglCreatePixmapSurface,
    _eglDestroySurface,
    _eglQuerySurface,
    _eglBindAPI,
    _eglQueryAPI,
    _eglWaitClient,
    _eglReleaseThread,
    _eglCreatePbufferFromClientBuffer,
    _eglSurfaceAttrib,
    _eglBindTexImage,
    _eglReleaseTexImage,
    _eglSwapInterval,
    _eglCreateContext,
    _eglDestroyContext,
    _eglMakeCurrent,
    _eglGetCurrentContext,
    _eglGetCurrentSurface,
    _eglGetCurrentDisplay,
    _eglQueryContext,
    _eglWaitGL,
    _eglWaitNative,
    _eglSwapBuffers,
    _eglCopyBuffers
};
