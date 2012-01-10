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

struct GVLCKlock {
    pthread_mutex_t mutex;
};

int
gvlckCreate(GVLCKlockptr *newLock)
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
    (*newLock)->mutex = mutex;

    return 0;
}

int
gvlckAcquire(GVLCKlockptr lock)
{
    if (pthread_mutex_lock(&(lock->mutex)) != 0)
    {
	perror("pthread_mutex_lock");
	return -1;
    }

    return 0;
}

int
gvlckTryToAcquire(GVLCKlockptr lock)
{
    if (pthread_mutex_trylock(&(lock->mutex)) != 0)
    {
	perror("pthread_mutex_trylock");
	return -1;
    }

    return 0; 
}

int
gvlckRelease(GVLCKlockptr lock)
{
    if (pthread_mutex_unlock(&(lock->mutex)) != 0)
    {
	perror("pthread_mutex_unlock");
	return -1;
    }

    return 0;
}

int
gvlckDestroy(GVLCKlockptr lock)
{
    if (pthread_mutex_destroy(&(lock->mutex)) != 0)
    {
	perror("pthread_mutex_destroy");
	return -1;
    }

    free(lock);

    return 0;
}
