/*! ***************************************************************************
 * \file    server_serializer.0.c
 * \brief
 * 
 * \date    December 20, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include "error.h"
#include "server_serializer.h"
#include "sleep.h"

/* ***************************************************************************
 * Server serializer implementation
 */

GVcmdid
gvStartReceiving(GVtransportptr transport,
		 GVlockptr      lock)
{
    GVcmdid  cmdId;

    TRY
    {
	if (gvRead(transport->callBuffer, &cmdId, sizeof(GVcmdid)) == -1)
	{
	    THROW(e0, "gvRead");
	}
    }
    CATCH (e0)
    {
	return -1;
    }
    
    return cmdId;
}

int
gvStopReceiving(GVtransportptr transport,
		GVlockptr      lock)
{
    return 0;
}

int
gvStartSending(GVtransportptr transport, 
	       GVlockptr      lock,
	       GVcallid       callId)
{
    
    TRY 
    {
	if (gvWrite(transport->returnBuffer, &callId, sizeof(GVcallid)) == -1)
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

int
gvStopSending(GVtransportptr transport,
	      GVlockptr      lock)
{
    return 0;
}

int
gvReceiveData(GVtransportptr  transport,
	      void           *toAddr,
	      size_t          length)
{
    TRY
    {
	if (gvRead(transport->callBuffer, toAddr, length) == -1)
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
gvSendData(GVtransportptr  transport,
	   const void     *fromAddr,
	   size_t          length)
{
    TRY
    {
	if (gvWrite(transport->returnBuffer, fromAddr, length) == -1)
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
