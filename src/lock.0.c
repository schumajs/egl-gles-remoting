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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "lock.h"

int
gvCreateLock(GVlockptr *newLock)
{
    pthread_mutex_t     mutex;
    pthread_mutexattr_t mutexAttr;

    pthread_mutexattr_init(&mutexAttr);
    pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
    if (pthread_mutexattr_init(&mutexAttr) != 0)
    {
	perror("pthread_mutexattr_init");
	return -1;
    }

    if (pthread_mutex_init(&mutex, &mutexAttr) != 0)
    {
	perror("pthread_mutex_init");
	return -1;
    }

    *newLock = malloc(sizeof(pthread_mutex_t));
    *(*newLock) = mutex;

    return 0;
}

int
gvlckAcquire(GVlockptr lock)
{
    if (pthread_mutex_lock(lock) != 0)
    {
	perror("pthread_mutex_lock");
	return -1;
    }

    return 0;
}

int
gvTryToAcquireLock(GVlockptr lock)
{
    if (pthread_mutex_trylock(lock) != 0)
    {
	perror("pthread_mutex_trylock");
	return -1;
    }

    return 0; 
}

int
gvReleaseLock(GVlockptr lock)
{
    if (pthread_mutex_unlock(lock) != 0)
    {
	perror("pthread_mutex_unlock");
	return -1;
    }

    return 0;
}

int
gvlckDestroy(GVlockptr lock)
{
    if (pthread_mutex_destroy(lock) != 0)
    {
	perror("pthread_mutex_destroy");
	return -1;
    }

    free(lock);

    return 0;
}
