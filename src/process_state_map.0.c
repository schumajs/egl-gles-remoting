/*! ***************************************************************************
 * \file    process_state_map.h
 * \brief   
 * 
 * \date    January 12, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <errno.h>
#include <pthread.h>
#include <uthash.h>

#include "error.h"
#include "process_state_map.h"
#include "transport.h" 

/* ****************************************************************************
 * Process state map implementation
 */

struct Item {
    unsigned long int  key;
    void              *value;
    UT_hash_handle     hh;
};

static struct Item      *hashtable     = NULL;
static pthread_rwlock_t  hashtableLock = PTHREAD_RWLOCK_INITIALIZER;

int
gvDelProcessState(unsigned long int key)
{
    struct Item *item = NULL;

    TRY
    {
	if (pthread_rwlock_wrlock(&hashtableLock) != 0)
	{
	    THROW(e0, "pthread_rwlock_wrlock");
	}

	HASH_FIND(hh, hashtable, &key, sizeof(unsigned long int), item);

	if (item == NULL)
	{
	    errno = EINVAL;
	    THROW(e0, "no such item");
	}

	HASH_DELETE(hh, hashtable, item);

	free(item);

	if (pthread_rwlock_unlock(&hashtableLock) != 0)
	{
	    THROW(e0, "pthread_rwlock_unlock");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvGetProcessState(unsigned long int   key,
		  void              **value)
{
    struct Item *item = NULL;
    
    TRY
    {
    	if (pthread_rwlock_rdlock(&hashtableLock) != 0)
	{
	    THROW(e0, "pthread_rwlock_rdlock");
	}

	HASH_FIND(hh, hashtable, &key, sizeof(unsigned long int), item);
	
	if (item == NULL)
	{
	    errno = EINVAL;
	    THROW(e0, "no such item");
	}

	*value = item->value;

	if (pthread_rwlock_unlock(&hashtableLock) != 0)
	{
	    THROW(e0, "pthread_rwlock_unlock");
	}
    }
    CATCH (e0)
    {
	return -1;
    }
    
    return 0;
}

int
gvPutProcessState(unsigned long int  key,
		  void              *value)
{
    struct Item *item;

    TRY
    {
	if (pthread_rwlock_wrlock(&hashtableLock) != 0)
	{
	    THROW(e0, "pthread_rwlock_wrlock");
	}

	if ((item = malloc(sizeof(struct Item))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	item->key   = key;
	item->value = value;

	HASH_ADD(hh, hashtable, key, sizeof(unsigned long int), item);

	if (pthread_rwlock_unlock(&hashtableLock) != 0)
	{
	    THROW(e0, "pthread_rwlock_unlock");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}
