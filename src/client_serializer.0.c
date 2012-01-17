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

#include "error.h"
#include "serializer.h"
#include "sleep.h"

/* ***************************************************************************
 * Client serializer implementation
 */

#define getCantorPair(k1, k2) \
    0.5 * ((k1 + k2) * (k1 + k2 + 1) + k2)

int
gvCall(GVtransportptr  transport,
       GVlockptr       lock,
       GVcmdid        *cmdId,
       GVcallid       *callId)
{
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

	    *callId = getCantorPair(pid, tid);
	}

	if (gvWrite(transport->callBuffer, cmdId, sizeof(GVcmdid)) == -1)
	{
	    THROW(e1, "gvWrite");
	}

	if (gvWrite(transport->callBuffer, callId, sizeof(GVcallid)) == -1)
	{
	    THROW(e1, "gvWrite");
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
gvEndCall(GVtransportptr transport,
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
gvReturn(GVtransportptr  transport, 
	 GVlockptr       lock,
	 GVcallid       *callId)
{
    void           *dataPtr;
    size_t          dataLength;

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

	    if (gvDataLength(transport->returnBuffer, &dataLength) == -1)
            {
		THROW(e1, "gvDataLength");
	    }

	    if (dataLength >= sizeof(GVcallid))
	    {
		if (gvDataPtr(transport->returnBuffer, &dataPtr) == -1)
		{
		    THROW(e1, "gvDataPtr");
		}
	    
		if (*((GVcallid *)dataPtr) == *callId)
		{
		    if (gvTake(transport->returnBuffer, sizeof(GVcallid)) == -1)
		    {
			THROW(e1, "gvTake");
		    }

		    break;
		}
	    }

	    if (lock != NULL)
	    {
		if (gvRelease(lock) == -1)
		{
		    THROW(e1, "gvRelease");
		}
	    }

	    gvSleep(0, 1000);
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
gvEndReturn(GVtransportptr transport,
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
gvGetData(GVtransportptr  transport,
	  void           *data,
	  size_t          length)
{
    TRY
    {
	if (gvRead(transport->returnBuffer, data, length) == -1)
	{
	    THROW(e0, "gvRead");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvPutData(GVtransportptr  transport,
          const void     *data,
	  size_t          length)
{
    TRY
    {
	if (gvWrite(transport->callBuffer, data, length) == -1)
	{
	    THROW(e0, "gvWrite");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}
