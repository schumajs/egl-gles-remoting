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

/* ****************************************************************************
 * Dispatching functions
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
    gvserReturnValue(&error, sizeof(EGLint));
    gvserEndReturn();
}

/* ****************************************************************************
 * Jump table
 */

GVDISfunc eglGlesJumpTable[1] = {
    _eglGetError
};
