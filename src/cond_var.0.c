/*! ***************************************************************************
 * \file    cond_var.0.c
 * \brief   
 * 
 * \date    January 20, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <stdlib.h>

#include "cond_var.h"
#include "error.h"

GVcondvarptr
gvCreateCondVar(void *desiredAddr)
{
    pthread_cond_t     *cond;
    pthread_condattr_t  condAttr;

    TRY
    {
	if (pthread_condattr_init(&condAttr) != 0)
	{
	    THROW(e0, "pthread_condattr_init");
	}

	if (pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED) != 0)
	{
	    THROW(e0, "pthread_condattr_setpshared");
	}

	if (desiredAddr == NULL)
	{
	    if ((cond = malloc(sizeof(pthread_cond_t))) == NULL)
	    {
		THROW(e0, "malloc");
	    }
	}
	else
	{
	    cond = desiredAddr;
	}

	if (pthread_cond_init(cond, &condAttr) != 0)
	{
	    THROW(e0, "pthread_cond_init");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return cond;
}

int
gvWait(GVcondvarptr condVar, GVlockptr lock)
{
    TRY
    {
	if (pthread_cond_wait(condVar, lock) != 0)
	{
	    THROW(e0, "pthread_cond_wait");
	}
    }
    CATCH(e0)
    {
	return -1;
    }

    return 0;
}

int
gvNotify(GVcondvarptr condVar)
{
    TRY
    {
	if (pthread_cond_signal(condVar) != 0)
	{
	    THROW(e0, "pthread_cond_signal");
	}
    }
    CATCH(e0)
    {
	return -1;
    }

    return 0;
}

int
gvDestroyCondVar(GVcondvarptr condVar)
{
    TRY
    {
	if (pthread_cond_destroy(condVar) != 0)
	{
	    THROW(e0, "pthread_cond_destroy");
	}

	free(condVar);
    }
    CATCH(e0)
    {
	return -1;
    }

    return 0;
}
