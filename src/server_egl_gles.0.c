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
    size_t          queryStringLength;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    gvReceiveData(transport, &name, sizeof(EGLint));

    queryString = eglQueryString(display, name);
    queryStringLength = strlen(queryString);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &queryStringLength, sizeof(size_t));
    gvSendData(transport, queryString, queryStringLength * sizeof(char));
}

void
_eglGetConfigs()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    EGLBoolean      status;
    EGLDisplay      display;
    EGLConfig      *configs = NULL;
    int             configsNull;
    EGLint          configSize;
    EGLint          numConfig;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    gvReceiveData(transport, &configsNull, sizeof(int));
    gvReceiveData(transport, &configSize, sizeof(EGLint));
    
    if (!configsNull)
    {
	configs = malloc(configSize * sizeof(EGLConfig));
    }

    status = eglGetConfigs(display, configs, configSize, &numConfig);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &status, sizeof(EGLBoolean));
    gvSendData(transport, &numConfig, sizeof(EGLint));

    if (!configsNull)
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
    int             attribListSize;
    EGLConfig      *configs;
    EGLint          configSize;
    EGLint          numConfig;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    gvReceiveData(transport, &attribListSize, sizeof(int));

    attribList = malloc(attribListSize * sizeof(EGLint));

    gvReceiveData(transport, attribList, attribListSize * sizeof(EGLint));
    gvReceiveData(transport, &configSize, sizeof(EGLint));

    configs = malloc(configSize * sizeof(EGLConfig));

    status = eglChooseConfig(display, attribList, configs, configSize, &numConfig);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &status, sizeof(EGLBoolean));
    gvSendData(transport, &numConfig, sizeof(EGLint));
    gvSendData(transport, configs, numConfig * sizeof(EGLConfig));
    
    free(attribList);
    free(configs);
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
    int                  attribListSize;
    EGLint              *attribList;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    gvReceiveData(transport, &config, sizeof(EGLConfig));
    gvReceiveData(transport, &window, sizeof(EGLNativeWindowType));
    gvReceiveData(transport, &attribListSize, sizeof(int));

    attribList = malloc(attribListSize * sizeof(EGLint));

    gvReceiveData(transport, attribList, attribListSize * sizeof(EGLint));

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
