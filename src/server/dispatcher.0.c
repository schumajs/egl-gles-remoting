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

#include "sleep.h"

#include "server/dispatcher.h"
#include "server/serializer.h"

/* ***************************************************************************
 * ThreadArg
 */

struct ThreadArg {
    GVDISdispatcherptr  dispatcher;
    GVDISfunc       *jumpTable;
};

#define getThreadArg(threadArg)  \
    ((struct ThreadArg *)threadArg)

#define getDispatcher(threadArg) \
    getThreadArg(threadArg)->dispatcher

#define getJumpTable(threadArg)  \
    getThreadArg(threadArg)->jumpTable

/* ***************************************************************************
 * Server dispatcher implementation
 */

#define getCallBuffer()   \
    ((GVDISdispatcherptr) gvdisGetCurrent())->transport->callBuffer

#define getReturnBuffer() \
    ((GVDISdispatcherptr) gvdisGetCurrent())->transport->returnBuffer

static void
*dispatchLoopThread(void *threadArg)
{
    GVSERcmdid cmdId;

    if (gvdisMakeCurrent(getDispatcher(threadArg)) == -1)
    {
	perror("gvdisMakeCurrent");
	return NULL;
    }

    /* The actual loop */
    while (1)
    {
	gvtrpRead(getCallBuffer(), &cmdId, sizeof(GVSERcmdid));
	getJumpTable(threadArg)[cmdId]();
	gvslpSleep(0, 1000);
    }

    return NULL;
}

int
gvdisDispatchLoop(GVDISdispatcherptr dispatcher, GVDISfunc jumpTable[], int join)
{
    pthread_t         thread;
    struct ThreadArg *threadArg;

    if ((threadArg = malloc(sizeof(struct ThreadArg))) == NULL)
    {
	perror("malloc");
	return -1;
    }

    if (dispatcher == NULL)
    {
	/* Inherit dispatcher of parent thread */
	dispatcher = gvdisGetCurrent();
    }

    threadArg->dispatcher = dispatcher;
    threadArg->jumpTable  = jumpTable;

    if (pthread_create(&thread, NULL, dispatchLoopThread, threadArg) != 0)
    {
	perror("pthread_create");
	return -1;
    }

    if (join)
    {
	if (pthread_join(thread, NULL))
	{
	    perror("pthread_join");
	    return -1;
	}
    }

    return 0;
}

/* ***************************************************************************
 * Server serializer implementation
 */

GVSERcallid
gvserCall()
{
    GVSERcallid callId;

    gvtrpRead(getCallBuffer(), &callId, sizeof(GVSERcallid));

    return callId;
}

void
gvserInData(void *data, size_t length)
{
    gvtrpRead(getCallBuffer(), data, length);
}

void
gvserEndCall()
{

}

void
gvserReturn(GVSERcallid callId)
{   
    gvtrpWrite(getReturnBuffer(), &callId, sizeof(GVSERcallid));
}

void
gvserOutData(const void *data, size_t length)
{
    gvtrpWrite(getReturnBuffer(), data, length);
}

void
gvserEndReturn()
{

}
