/*! ***************************************************************************
 * \file    thread_state_map.0.c
 * \brief
 * 
 * \date    January 9, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#define _MULTI_THREADED
#include <pthread.h>
#include <uthash.h>

#include "error.h"
#include "thread_state_map.h"

/* ***************************************************************************
 * Thread state map implementation
 */

struct Item {
    unsigned long int  key;
    void              *value;
    UT_hash_handle     hh;
};

static pthread_mutex_t initTerminateLock = PTHREAD_MUTEX_INITIALIZER;
static int             threadRefCounter  =  0;
static pthread_key_t   threadSpecificKey = -1;

#define getThreadSpecificData() \
    pthread_getspecific(threadSpecificKey)

#define setThreadSpecificData(data) \
    pthread_setspecific(threadSpecificKey, data)

static void
threadSpecificDataDestructor(void *threadSpecificData) {
    pthread_setspecific(threadSpecificKey, NULL);
}

int
gvInitThreadStateMap()
{
    TRY
    {
	if (pthread_mutex_lock(&initTerminateLock) != 0)
	{
	    THROW(e0, "pthread_mutex_lock");
	}

	if (threadSpecificKey == -1)
	{
	    if (pthread_key_create(&threadSpecificKey,
				   threadSpecificDataDestructor) != 0)
	    {
		THROW(e1, "pthread_key_create");
	    }
	}
    
	threadRefCounter++;

	if (pthread_mutex_unlock(&initTerminateLock) != 0)
	{
	    THROW(e1, "pthread_mutex_unlock");
	}
    }
    CATCH (e0)
    {
	return -1;
    }
    CATCH (e1)
    {
	/* Try to unlock. At this point we can't do anything if unlocking
         * fails, so just ignore errors.
         */
	pthread_mutex_unlock(&initTerminateLock);
	return -1;
    }
   
    return 0;
}

int
gvDelThreadState(unsigned long key)
{
    struct Item *item = NULL;
    struct Item *hashtable;

    TRY
    {
	if ((hashtable = (struct Item *)getThreadSpecificData()) == NULL)
	{
	    THROW(e0, "no thread state");
	}

	HASH_FIND(hh, hashtable, &key, sizeof(unsigned long int), item);

	if (item == NULL)
	{
	    THROW(e0, "no such item");
	}

	HASH_DELETE(hh, hashtable, item);

	free(item);
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

void
*gvGetThreadState(unsigned long int key)
{
    struct Item *item = NULL;
    struct Item *hashtable;

    TRY
    {
	if ((hashtable = (struct Item *)getThreadSpecificData()) == NULL)
	{
	    THROW(e0, "no thread state");
	}

	HASH_FIND(hh, hashtable, &key, sizeof(unsigned long int), item);

	if (item == NULL)
	{
	    THROW(e0, "no such item");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return item->value;
}

int
gvPutThreadState(unsigned long int  key,
		 void              *value)
{
    struct Item *item;
    struct Item *hashtable;

    TRY
    {
	if ((item = malloc(sizeof(struct Item))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	item->key   = key;
	item->value = value;

	if ((hashtable = (struct Item *)getThreadSpecificData()) != NULL)
	{	 
	    HASH_ADD(hh, hashtable, key, sizeof(unsigned long int), item);
	}
	else
	{
	    HASH_ADD(hh, hashtable, key, sizeof(unsigned long int), item);

	    if (setThreadSpecificData(hashtable) != 0)
	    {
		THROW(e0, "pthread_setspecific");
	    }
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvTermThreadStateMap()
{
    TRY
    {
	if (pthread_mutex_lock(&initTerminateLock) != 0)
	{
	    THROW(e0, "pthread_mutex_lock");
	}

	if (threadRefCounter == 1)
	{
	    if (pthread_key_delete(threadSpecificKey) != 0)
	    {
		THROW(e1, "pthread_key_delete");
	    }
	
	    threadSpecificKey = -1;
	}

	threadRefCounter--;

	if (pthread_mutex_unlock(&initTerminateLock) != 0)
	{
	    THROW(e1, "pthread_mutex_unlock");
	}
    }
    CATCH (e0)
    {
	return -1;
    }
    CATCH (e1)
    {
	/* Try to unlock. At this point we can't do anything if unlocking
         * fails, so just ignore errors.
         */
	pthread_mutex_unlock(&initTerminateLock);
	return -1;
    } 

    return 0;
}
