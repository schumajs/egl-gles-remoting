/*! ***************************************************************************
 * \file    serializer.0.c
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
#include "serializer.h"

/* ***************************************************************************
 * Serializer implementation
 */

void
*gvReceiveVarSizeData(GVtransportptr transport)
{
    void   *toAddr = NULL;
    size_t  length;

    TRY
    {
	if (gvReceiveData(transport, &length, sizeof(size_t)) == -1)
	{
	    THROW(e0, "gvReceiveData");
	}

	
	if (length > 0)
	{
	    if ((toAddr = malloc(length)) == NULL)
	    {
		THROW(e0, "malloc");
	    }

	    if (gvReceiveData(transport, toAddr, length) == -1)
	    {
		THROW(e0, "gvReceiveData");
	    }
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return toAddr;
}

int
gvSendVarSizeData(GVtransportptr  transport,
		  const void     *fromAddr,
		  size_t          length)
{
    TRY
    {
	if (gvSendData(transport, &length, sizeof(size_t)) == -1)
	{
	    THROW(e0, "gvSendData");
	}

	if (length > 0)
	{
	    if (gvSendData(transport, fromAddr, length) == -1)
	    {
		THROW(e0, "gvSendData");
	    }
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

