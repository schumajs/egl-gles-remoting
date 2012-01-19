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

#include <stdio.h>
#include <stdlib.h>
#include <EGL/egl.h>

#include "server/dispatcher.h"
#include "server/serializer.h"

/* ***************************************************************************
 * Client / server coordination
 */

static int
notifyCreateContext(size_t offset, size_t length)
{
    /*
    GVTRPtransportptr  transport;
    GVDISdispatcherptr dispatcher;

    if (gvtrpCreate(&transport,
		    context->vmShm, offset, length) == -1)
    {
	perror("gvtrpCreate");
	return -1;
    }

    if ((gvdisCreate(&dispatcher, transport) == -1))
    {
	perror("gvdisCreate");
	exit(2);
    }

    if (gvdisDispatchLoop(dispatcher, eglGlesJumpTable, 0) == -1)
    {
	perror("gvdisDispatchLoop");
	exit(2);
    }
    */

    return 0;
}

static int
notifyDestroyContext()
{
    GVDISdispatcherptr dispatcher;

    if ((dispatcher = gvdisGetCurrent()) == NULL)
    {
	perror("gvdisGetCurrent");
	return -1;
    }

    /* Destroy transport */
    if (gvtrpDestroy(dispatcher->transport) == -1)
    {
	perror("gvtrpDestroy");
	return -1;
    }

    /* Destroy dispatcher */
    if (gvdisDestroy(dispatcher) == -1)
    {
	perror("gvdisDestroy");
	return -1;
    }    

    return 0;
}

/* ****************************************************************************
 * Dispatching functions
 */

/* ***************************************************************************
 * Client / server coordination
 */

static void
_notInUse()
{
    GVSERcallid callId;
    int         status;

    callId = gvserCall();
    gvserEndCall();

    status = -1;

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLint));
    gvserEndReturn();
}

static void
_notifyCreateContext()
{
    GVSERcallid callId;
    size_t      offset;
    size_t      length;
    int         status;

    callId = gvserCall(GVSER_RESERVED_0);
    gvserInData(&offset, sizeof(size_t));
    gvserInData(&length, sizeof(size_t));
    gvserEndCall();

    status = notifyCreateContext(offset, length);

    gvserReturn(callId);
    gvserOutData(&status, sizeof(int));
    gvserEndReturn();
}

static void
_notifyDestroyContext()
{
    GVSERcallid callId;    
    int         status;

    callId = gvserCall(GVSER_RESERVED_1);
    gvserEndCall();

    status = notifyDestroyContext();

    gvserReturn(callId);
    gvserOutData(&status, sizeof(int));
    gvserEndReturn();
}

/* ***************************************************************************
 * EGL
 */

static
void _eglGetError()
{
    GVSERcallid callId;
    EGLint      error;

    callId = gvserCall();
    gvserEndCall();

    error = eglGetError();

    gvserReturn(callId);
    gvserOutData(&error, sizeof(EGLint));
    gvserEndReturn();
}

static
void _eglGetDisplay()
{
    GVSERcallid          callId;
    EGLNativeDisplayType displayId;
    EGLDisplay           display;

    callId = gvserCall(GVSER_EGL_GETDISPLAY);
    gvserInData(&displayId, sizeof(EGLNativeDisplayType));
    gvserEndCall();

    display = eglGetDisplay(displayId);

    gvserReturn(callId);
    gvserOutData(&display, sizeof(EGLDisplay));
    gvserEndReturn();
}

static
void _eglInitialize()
{
    GVSERcallid callId;
    EGLDisplay  display;
    EGLint      minor;
    EGLint      major;
    EGLBoolean  status;

    callId = gvserCall(GVSER_EGL_INITIALIZE);
    gvserInData(&display, sizeof(EGLDisplay));
    gvserEndCall();

    status = eglInitialize(display, &minor, &major);

    gvserReturn(callId);
    gvserOutData(&status, sizeof(EGLBoolean));
    gvserOutData(&major,  sizeof(EGLint));
    gvserOutData(&minor,  sizeof(EGLint));
    gvserEndReturn();
}

/* ****************************************************************************
 * Jump table
 */

GVDISfunc eglGlesJumpTable[13] = {
    _notifyCreateContext,
    _notifyDestroyContext,
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
    _eglInitialize
};
