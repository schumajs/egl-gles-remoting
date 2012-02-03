/*! ***************************************************************************
 * \file    lock.c
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
#include "lock.h"

GVlockptr
gvCreateLock(void *desiredAddr)
{
    pthread_mutex_t     *mutex;
    pthread_mutexattr_t  mutexAttr;

    TRY
    {
	if (pthread_mutexattr_init(&mutexAttr) != 0)
	{
	    THROW(e0, "pthread_mutexattr_init");
	}

	if (pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED) != 0)
	{
	    THROW(e0, "pthread_mutexattr_setpshared");
	}

	if (desiredAddr == NULL)
	{
	    if ((mutex = malloc(sizeof(pthread_mutex_t))) == NULL)
	    {
		THROW(e0, "malloc");
	    }
	}
	else
	{
	    mutex = desiredAddr;
	}

	if (pthread_mutex_init(mutex, &mutexAttr) != 0)
	{
	    THROW(e0, "pthread_mutex_init");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return mutex;
}

int
gvAcquire(GVlockptr lock)
{
    TRY
    {
	if (pthread_mutex_lock(lock) != 0)
	{
	    THROW(e0, "pthread_mutex_lock");
	}
    }
    CATCH(e0)
    {
	return -1;
    }

    return 0;
}

int
gvTryToAcquire(GVlockptr lock)
{
    TRY
    {
	if (pthread_mutex_trylock(lock) != 0)
	{
	    THROW(e0, "pthread_mutex_trylock");
	}
    }
    CATCH(e0)
    {
	return -1;
    }

    return 0;
}

int
gvRelease(GVlockptr lock)
{
    TRY
    {
	if (pthread_mutex_unlock(lock) != 0)
	{
	    THROW(e0, "pthread_mutex_unlock");
	}
    }
    CATCH(e0)
    {
	return -1;
    }

    return 0;
}

int
gvDestroyLock(GVlockptr lock)
{
    TRY
    {
	if (pthread_mutex_destroy(lock) != 0)
	{
	    THROW(e0, "pthread_mutex_destroy");
	}

	free(lock);
    }
    CATCH(e0)
    {
	return -1;
    }

    return 0;
}
