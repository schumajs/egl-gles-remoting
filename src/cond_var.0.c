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
#include <string.h>

#include "cond_var.h"
#include "error.h"

GVcondvarptr
gvCreateCondVar()
{
    pthread_cond_t     cond;
    pthread_condattr_t condAttr;
    GVlockptr          lock;
    GVcondvarptr       newCondVar;

    TRY
    {
	if ((lock = gvCreateLock()) == NULL)
	{
	    THROW(e0, "gvCreateLock");
	}

	if (pthread_condattr_init(&condAttr) != 0)
	{
	    THROW(e0, "pthread_condattr_init");
	}

	if (pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED) != 0)
	{
	    THROW(e0, "pthread_condattr_setpshared");
	}

	if (pthread_cond_init(&cond, &condAttr) != 0)
	{
	    THROW(e0, "pthread_cond_init");
	}

	if ((newCondVar = malloc(sizeof(struct GVcondvar))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	if ((newCondVar->cond = malloc(sizeof(pthread_cond_t))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	newCondVar->lock = lock;
	memcpy(newCondVar->cond, &cond, sizeof(pthread_cond_t));
    }
    CATCH (e0)
    {
	return NULL;
    }

    return newCondVar;
}

int
gvWait(GVcondvarptr condVar)
{
    TRY
    {
	if (pthread_cond_wait(condVar->cond, condVar->lock) != 0)
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
	printf("%p\n", condVar->cond);

	if (pthread_cond_broadcast(condVar->cond) != 0)
	{
	    THROW(e0, "pthread_cond_signal");
	}

	puts("CHECKPOINT");
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
	if (gvDestroyLock(condVar->lock) == -1)
	{
	    THROW(e0, "gvDestroyLock");
	}

	if (pthread_cond_destroy(condVar->cond) != 0)
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
