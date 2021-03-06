/*! ***************************************************************************
 * \file    client_serializer.0.c
 * \brief
 * 
 * \date    December 20, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <unistd.h>

#include "client_serializer.h"
#include "error.h"
#include "sleep.h"

/* ***************************************************************************
 * Client serializer implementation
 */

/* TODO replace cantor pairing: slow, needs bignum */
#define getCantorPair(k1, k2) \
    0.5 * ((k1 + k2) * (k1 + k2 + 1) + k2)

GVcallid
gvStartSending(GVtransportptr  transport,
	       GVlockptr       lock,
	       GVcmdid         cmdId)
{
    GVcallid callId;

    TRY
    {
	if (lock != NULL)
	{
	    if (gvAcquireLock(lock) == -1)
	    {
		THROW(e0, "gvAcquireLock");
	    }
	}

	/* TODO replace cantor pairing function, pthread_self is UL,
	 * syscall(SYS_gettid) is TID!
	 */
	{
	    pid_t     pid = getpid();
	    pthread_t tid = pthread_self();

	    callId = getCantorPair(pid, tid);
	}

	if (transport->write(transport->callChanel, &cmdId, sizeof(GVcmdid)) == -1)
	{
	    THROW(e1, "write");
	}

	if (transport->write(transport->callChanel, &callId, sizeof(GVcallid)) == -1)
	{
	    THROW(e1, "write");
	}
    }
    CATCH (e0)
    {
	return -1;
    }
    CATCH (e1)
    {
	if (lock != NULL)
	{
	    /* Try to unlock. At this point we can't do anything if unlocking
	     * fails, so just ignore errors.
	     */
	    gvReleaseLock(lock);
	}

	return -1;
    }
    
    return callId;
}

int
gvStopSending(GVtransportptr transport,
	      GVlockptr      lock)
{
    TRY
    {
	if (lock != NULL)
	{
	    if (gvReleaseLock(lock) == -1)
	    {
		THROW(e0, "gvReleaseLock");
	    }
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvStartReceiving(GVtransportptr transport, 
		 GVlockptr      lock,
		 GVcallid       callId)
{
    TRY
    {
	while (1)
	{
	    if (lock != NULL)
	    {
		if (gvAcquireLock(lock) == -1)
		{
		    THROW(e0, "gvAcquireLock");
		}
	    }

	    if (*((GVcallid *)transport->peek(transport->returnChanel,
					      sizeof(GVcallid))) == callId)
	    {
		if (transport->take(transport->returnChanel, sizeof(GVcallid)) == -1)
		{
		    THROW(e1, "skip");
		}

		break;
	    }

	    if (lock != NULL)
	    {
		if (gvReleaseLock(lock) == -1)
		{
		    THROW(e1, "gvReleaseLock");
		}
	    }
        }
    }
    CATCH (e0)
    {
	return -1;
    }
    CATCH (e1)
    {
	if (lock != NULL)
	{
	    /* Try to unlock. At this point we can't do anything if unlocking
	     * fails, so just ignore errors.
	     */
	    gvReleaseLock(lock);
	}

	return -1;
    }

    return 0;
}

int
gvStopReceiving(GVtransportptr transport,
		GVlockptr      lock)
{
    TRY
    {
	if (lock != NULL)
	{
	    if (gvReleaseLock(lock) == -1)
	    {
		THROW(e0, "gvReleaseLock");
	    }
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvReceiveData(GVtransportptr  transport,
	      void           *toAddr,
	      size_t          length)
{
    TRY
    {
	if (transport->read(transport->returnChanel, toAddr, length) == -1)
	{
	    THROW(e0, "read");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvSendData(GVtransportptr  transport,
	   const void     *fromAddr,
	   size_t          length)
{
    TRY
    {
	if (transport->write(transport->callChanel, fromAddr, length) == -1)
	{
	    THROW(e0, "write");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

