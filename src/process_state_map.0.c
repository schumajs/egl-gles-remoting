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

#include <pthread.h>
#include <uthash.h>

#include "error.h"
#include "process_state_map.h"
#include "transport.h" 

/* ****************************************************************************
 * Process state map implementation
 */

struct Item {
    void           *valPtr;
    UT_hash_handle  hh;
};

static struct Item      *hashtable     = NULL;
static pthread_rwlock_t  hashtableLock = PTHREAD_RWLOCK_INITIALIZER;

int
gvDelProcessState(void   *keyPtr,
                  size_t  keyLength)
{
    struct Item *item;

    TRY
    {
	if (pthread_rwlock_wrlock(&hashtableLock) != 0)
	{
	    THROW(e0, "pthread_rwlock_wrlock");
	}

	HASH_FIND(hh, hashtable, keyPtr, keyLength, item);
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
gvGetProcessState(void    *keyPtr,
		  size_t   keyLength,
		  void   **valPtr)
{
    struct Item *item;
    
    TRY
    {
    	if (pthread_rwlock_rdlock(&hashtableLock) != 0)
	{
	    THROW(e0, "pthread_rwlock_rdlock");
	}

	puts("AAA");

	printf("GET COUNT1 %i\n", HASH_COUNT(hashtable));

	printf("GETTING %zu\n", *((size_t *)keyPtr));

	HASH_FIND(hh, hashtable, keyPtr, keyLength, item);
	
	puts("HIER");

	printf("ITEM %p", item);

	*valPtr = item->valPtr;

	puts("AAB");

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
gvPutProcessState(void   *keyPtr,
		  size_t  keyLength,
		  void   *valPtr)
{
    struct Item *item;

    TRY
    {
	if ((item = malloc(sizeof(struct Item))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	if (pthread_rwlock_wrlock(&hashtableLock) != 0)
	{
	    THROW(e0, "pthread_rwlock_wrlock");
	}

	printf("PUT COUNT1 %i\n", HASH_COUNT(hashtable));

	printf("PUTTING %zu\n", *((size_t *)keyPtr));

	item->valPtr = valPtr;
	HASH_ADD_KEYPTR(hh, hashtable, keyPtr, keyLength, item);

	printf("PUT COUNT2 %i\n", HASH_COUNT(hashtable));

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
