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
	    if (gvAcquire(lock) == -1)
	    {
		THROW(e0, "gvAcquire");
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

	if (transport->write(transport->oc, &cmdId, sizeof(GVcmdid)) == -1)
	{
	    THROW(e1, "write");
	}

	if (transport->write(transport->oc, &callId, sizeof(GVcallid)) == -1)
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
	    gvRelease(lock);
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
	    if (gvRelease(lock) == -1)
	    {
		THROW(e0, "gvRelease");
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
		if (gvAcquire(lock) == -1)
		{
		    THROW(e0, "gvAcquire");
		}
	    }

	    if (*((GVcallid *)transport->peek(transport->ic,
					      sizeof(GVcallid))) == callId)
	    {
		if (transport->take(transport->ic, sizeof(GVcallid)) == -1)
		{
		    THROW(e1, "skip");
		}

		break;
	    }

	    if (lock != NULL)
	    {
		if (gvRelease(lock) == -1)
		{
		    THROW(e1, "gvRelease");
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
	    gvRelease(lock);
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
	    if (gvRelease(lock) == -1)
	    {
		THROW(e0, "gvRelease");
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
	if (transport->read(transport->ic, toAddr, length) == -1)
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
	if (transport->write(transport->oc, fromAddr, length) == -1)
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

