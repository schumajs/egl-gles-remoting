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

#include <stdlib.h>

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
	if (transport->read(transport->oc, &cmdId, sizeof(GVcmdid)) == -1)
	{
	    THROW(e0, "read");
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
	if (transport->write(transport->ic, &callId, sizeof(GVcallid)) == -1)
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
	if (transport->read(transport->oc, toAddr, length) == -1)
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
	if (transport->write(transport->ic, fromAddr, length) == -1)
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
