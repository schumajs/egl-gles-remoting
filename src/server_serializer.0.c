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
#include "serializer.h"
#include "sleep.h"

/* ***************************************************************************
 * Server serializer implementation
 */

int
gvCall(GVtransportptr   transport,
       GVlockptr        lock,
       GVcmdid          *cmdId,
       GVcallid         *callId)
{
    TRY
    {
	if (gvRead(transport->callBuffer, callId, sizeof(GVcallid)) == -1)
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
gvEndCall(GVtransportptr transport,
	  GVlockptr      lock)
{
    return 0;
}

int
gvReturn(GVtransportptr  transport, 
	 GVlockptr       lock,
	 GVcallid       *callId)
{
    TRY 
    {
	if (gvWrite(transport->returnBuffer, callId, sizeof(GVcallid)) == -1)
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
gvEndReturn(GVtransportptr transport,
	    GVlockptr      lock)
{
    return 0;
}

int
gvGetData(GVtransportptr  transport,
	  void           *data,
	  size_t          length)
{
    TRY
    {
	if (gvRead(transport->callBuffer, data, length) == -1)
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
	if (gvWrite(transport->returnBuffer, data, length) == -1)
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
