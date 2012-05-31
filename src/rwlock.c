/*! ***************************************************************************
 * \file    rwlock.c
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
#include "rwlock.h"

GVrwlockptr
gvCreateRwLock(void *desiredAddr)
{
    pthread_rwlock_t     *rwlock;
    pthread_rwlockattr_t  rwlockAttr;

    TRY
    {
	if (pthread_rwlockattr_init(&rwlockAttr) != 0)
	{
	    THROW(e0, "pthread_rwlockattr_init");
	}

	if (pthread_rwlockattr_setpshared(&rwlockAttr, PTHREAD_PROCESS_SHARED) != 0)
	{
	    THROW(e0, "pthread_rwlockattr_setpshared");
	}

	if (desiredAddr == NULL)
	{
	    if ((rwlock = malloc(sizeof(pthread_rwlock_t))) == NULL)
	    {
		THROW(e0, "malloc");
	    }
	}
	else
	{
	    rwlock = desiredAddr;
	}

	if (pthread_rwlock_init(rwlock, &rwlockAttr) != 0)
	{
	    THROW(e0, "pthread_rwlock_init");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return rwlock;
}

int
gvAcquireReadLock(GVrwlockptr lock)
{
    TRY
    {
	if (pthread_rwlock_rdlock(lock) != 0)
	{
	    THROW(e0, "pthread_rwlock_rdlock");
	}
    }
    CATCH(e0)
    {
	return -1;
    }

    return 0;
}

int
gvAcquireWriteLock(GVrwlockptr lock)
{
    TRY
    {
	if (pthread_rwlock_wrlock(lock) != 0)
	{
	    THROW(e0, "pthread_rwlock_wrlock");
	}
    }
    CATCH(e0)
    {
	return -1;
    }

    return 0;
}

int
gvReleaseRwLock(GVrwlockptr lock)
{
    TRY
    {
	if (pthread_rwlock_unlock(lock) != 0)
	{
	    THROW(e0, "pthread_rwlock_unlock");
	}
    }
    CATCH(e0)
    {
	return -1;
    }

    return 0;
}

int
gvDestroyRwLock(GVrwlockptr lock)
{
    TRY
    {
	if (pthread_rwlock_destroy(lock) != 0)
	{
	    THROW(e0, "pthread_rwlock_destroy");
	}

	free(lock);
    }
    CATCH(e0)
    {
	return -1;
    }

    return 0;
}
