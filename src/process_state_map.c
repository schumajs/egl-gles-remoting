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

static struct Item *hashtable = NULL;

int
gvDelProcessStateItem(unsigned long int key)
{
    struct Item *item = NULL;

    TRY
    {
	HASH_FIND(hh, hashtable, &key, sizeof(unsigned long int), item);

	if (item == NULL)
	{
	    errno = EINVAL;
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
*gvGetProcessStateItem(unsigned long int key)
{
    struct Item *item = NULL;
    
    TRY
    {
	HASH_FIND(hh, hashtable, &key, sizeof(unsigned long int), item);
	
	if (item == NULL)
	{
	    errno = EINVAL;
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
gvForeachProcessStateItem(GVpstateiterfunc  func,
			  void             *arg)
{
    struct Item *item, *tempItem;

    HASH_ITER(hh, hashtable, item, tempItem) {
	func(item->key, item->value, arg);
    }

    return 0;
}

int
gvPutProcessStateItem(unsigned long int  key,
		      void              *value)
{
    struct Item *item;

    TRY
    {
	if ((item = malloc(sizeof(struct Item))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	item->key   = key;
	item->value = value;

	HASH_ADD(hh, hashtable, key, sizeof(unsigned long int), item);
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}
