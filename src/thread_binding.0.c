/*! ***************************************************************************
 * \file    thread_binding.0.c
 * \brief   
 * 
 * \date    January 12, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <pthread.h>

#include "thread_binding.h" 

struct Binding {
    /* Public data*/
    GVbinding     public;

    /* Private data */
    pthread_key_t threadSpecificKey;
}; 

#define castBinding(binding) \
    ((struct Binding *)binding)

static void
threadSpecificDataDestructor(void *data)
{

}

int
gvCreateBinding(GVbindingptr *newBinding, void *data)
{
    struct Binding *binding;

    binding = malloc(sizeof(struct Binding));

    /* Init. public data */

    binding->public.data = data;

    /* Init. private data */

    if (pthread_key_create(&binding->priv.threadSpecificKey,
			   threadSpecificDataDestructor) != 0)
    {
	perror("pthread_key_create");
	return -1;
    }

    *newBinding	= (GVbindingptr) binding;

    return 0;
}

int
gvBind(GVbindingptr binding)
{

}

GVbindingptr
gvGetBinding(void)
{

}

int
gvUnbind(GVbindingptr binding)
{

}

int
gvDestroyBinding(GVbindingptr binding)
{

}
