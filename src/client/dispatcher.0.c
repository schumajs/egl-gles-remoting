/*! ***************************************************************************
 * \file    dispatcher.0.c
 * \brief
 * 
 * \date    January 9, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#define _MULTI_THREADED
#include <pthread.h>
#include <stdio.h>

#include "dispatcher.h"
#include "sleep.h"

#include "client/serializer.h"

/* ***************************************************************************
 * Client dispatcher implementation
 */

/* TODO cache current buffer */

#define getCallBuffer()   \
    ((GVDISdispatcherptr) gvdisGetCurrent())->transport->callBuffer

#define getReturnBuffer() \
    ((GVDISdispatcherptr) gvdisGetCurrent())->transport->returnBuffer

/* ***************************************************************************
 * Client serializer implementation
 */

#define getCantorPair(k1, k2) \
    0.5 * ((k1 + k2) * (k1 + k2 + 1) + k2)

GVSERcallid
gvserCall(GVSERcmdid cmdId)
{
    GVSERcallid callId;

    gvlckAcquire(getCallBuffer()->clientLock);

    /* TODO replace cantor pairing function */
    {
	pid_t     pid = getpid();
	pthread_t tid = pthread_self();

	callId = getCantorPair(pid, tid);
    }

    gvtrpWrite(getCallBuffer(), &cmdId, sizeof(GVSERcmdid));
    gvtrpWrite(getCallBuffer(), &callId, sizeof(GVSERcallid));

    return callId;
}

void
gvserInParameter(const void *data, size_t length)
{
    gvtrpWrite(getCallBuffer(), data, length);
}

void
gvserEndCall()
{
    gvlckRelease(getCallBuffer()->clientLock);
}

void
gvserReturn(GVSERcallid callId)
{
    void   *dataPtr;
    size_t  dataLength;

    while (1)
    {
	gvlckAcquire(getReturnBuffer()->clientLock);    

	gvtrpDataLength(getReturnBuffer(), &dataLength);

	if (dataLength >= sizeof(GVSERcallid))
	{
	    gvtrpDataPtr(getReturnBuffer(), &dataPtr);
	    if (*((GVSERcallid *)dataPtr) == callId)
	    {
		gvtrpTake(getReturnBuffer(), sizeof(GVSERcallid));
		break;
	    }
	}

	gvlckRelease(getReturnBuffer()->clientLock);

	gvslpSleep(0, 1000);
    }
}

void
gvserReturnValue(void *data, size_t length)
{
    gvtrpRead(getReturnBuffer(), data, length);
}

void
gvserOutParameter(void *data, size_t length)
{
    gvtrpRead(getReturnBuffer(), data, length);
}

void
gvserEndReturn()
{
    gvlckRelease(getReturnBuffer()->clientLock);    
}
