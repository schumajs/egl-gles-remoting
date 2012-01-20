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
    GVtransportptr transport = gvGetThreadTransport();

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
    GVtransportptr       transport = gvGetThreadTransport();

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
    GVtransportptr transport = gvGetThreadTransport();
    
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
    GVtransportptr transport = gvGetThreadTransport();

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
    GVtransportptr  transport = gvGetThreadTransport();

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

/* ****************************************************************************
 * Jump table
 */

GVdispatchfunc eglGlesJumpTable[15] = {
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
    _eglQueryString
};
