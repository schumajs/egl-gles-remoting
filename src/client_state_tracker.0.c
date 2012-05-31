/*! ***************************************************************************
 * \file    client_state_tracker.0.c
 * \brief
 * 
 * \date    January 9, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <uthash.h>

#include "client_state_tracker.h"
#include "error.h"
#include "process_state_map.h"
#include "thread_state_map.h"

/* ***************************************************************************
 * Client state tracker implementation
 */

#define CURRENT_DISPLAY_KEY            0x0
#define CURRENT_CONTEXT_KEY            0x1
#define CURRENT_TRANSPORT_KEY          0x2
#define VERTEX_ATTRIB_KEY              0x3
#define NUM_ENABLED_VERTEX_ATTRIBS_KEY 0x4
#define VERTEX_BUFFER_KEY              0x5

static pthread_rwlock_t processStateLock        = PTHREAD_RWLOCK_INITIALIZER;
static int              threadStateMapInitiated = 0;

/* ***************************************************************************
 * Initialization
 */

static int
initVertexAttribs()
{
    GVvertexattribptr *attribs;
    size_t             attribsSize = MAX_NUM_VERTEX_ATTRIBS
	                             * sizeof(struct GVvertexattrib);

    if ((attribs = malloc(attribsSize)) == NULL)
    {
	return -1;
    }

    memset(attribs, 0, attribsSize);

    if (gvPutThreadStateItem(VERTEX_ATTRIB_KEY, attribs) == -1)
    {
	return -1;
    }

    return 0;
}

static int
initNumEnabledVertexAttribs()
{
    int *num;

    if ((num = malloc(sizeof(int))) == NULL)
    {
	return -1;
    }

    *num = 0;

    if (gvPutThreadStateItem(NUM_ENABLED_VERTEX_ATTRIBS_KEY, num) == -1)
    {
	return -1;
    }

    if (gvPutThreadStateItem(NUM_ENABLED_VERTEX_ATTRIBS_KEY, num) == -1)
    {
	return -1;
    }
    return 0;
}

struct VertexBuffer {
    GLuint id;

    UT_hash_handle hh;
};

static int
initVertexBuffers()
{
    struct VertexBuffer **buffers;
    size_t                buffersSize = 2 * sizeof(struct VertexBuffer*);

    if ((buffers = malloc(buffersSize)) == NULL)
    {
	return -1;
    }
    
    memset(buffers, 0, buffersSize);

    if (gvPutThreadStateItem(VERTEX_BUFFER_KEY, buffers) == -1)
    {
	return -1;
    }

    return 0;
}

#define initIfNotDoneAlready()						\
    do {								\
	if (!threadStateMapInitiated)					\
	{								\
	    if (gvInitThreadStateMap() == -1)				\
	    {								\
		return -1;						\
									\
	    }								\
									\
	    if (initVertexAttribs() == -1)				\
	    {								\
		return -1;						\
	    }								\
									\
	    if (initNumEnabledVertexAttribs() == -1)			\
	    {								\
		return -1;						\
	    }								\
	    								\
	    if (initVertexBuffers() == -1)				\
	    {								\
		return -1;						\
	    }								\
	    								\
	    threadStateMapInitiated = 1;				\
	}								\
    } while (0)


/* ***************************************************************************
 * Context
 */

struct ContextIterFuncArg {
    EGLDisplay        display;
    GVcstateiterfunc  func;
    void             *arg;
};

static void
contextIterFunc(unsigned long int  key,
		void              *value,
		void              *arg)
{
    struct ContextIterFuncArg *iterFuncArg = (struct ContextIterFuncArg *)arg;
    GVcontextstateptr          state       = (GVcontextstateptr)value;
    
    if (state->display == iterFuncArg->display)
    {
	(iterFuncArg->func)(state, iterFuncArg->arg);
    }
}

static void
setAllMarkedDestroyedIterFunc(void *state,
			      void *arg)
{
    ((GVcontextstateptr) state)->markedDestroyed = *((int *)arg);
}

int
gvDelEglContextState(EGLDisplay display,
		     EGLContext context)
{
    TRY
    {
        if (pthread_rwlock_wrlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_wrlock");
        }

	if (gvDelProcessStateItem((size_t) context) == -1)
	{
	    THROW(e1, "gvDelProcessStateItem");
	}

        if (pthread_rwlock_unlock(&processStateLock) != 0)
        {
            THROW(e1, "pthread_rwlock_unlock");
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
        pthread_rwlock_unlock(&processStateLock);
        return -1;
    }

    return 0;
}

GVcontextstateptr
gvGetEglContextState(EGLDisplay display,
		     EGLContext context)
{
    GVcontextstateptr state;

    TRY
    {
        if (pthread_rwlock_rdlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_rdlock");
        }

	if ((state
	     = (GVcontextstateptr) gvGetProcessStateItem(
		 (size_t) context)) == NULL)
	{
	    THROW(e1, "gvGetProcessStateItem");
	}

        if (pthread_rwlock_unlock(&processStateLock) != 0)
        {
            THROW(e1, "pthread_rwlock_unlock");
        }
    }
    CATCH (e0)
    {
	return NULL;
    }
    CATCH (e1)
    {
        /* Try to unlock. At this point we can't do anything if unlocking
         * fails, so just ignore errors.
         */
        pthread_rwlock_unlock(&processStateLock);
        return NULL;
    }

    return state;
}

int
gvForeachEglContextState(EGLDisplay        display,
			 GVcstateiterfunc  func,
			 void             *arg)
{
    struct ContextIterFuncArg iterFuncArg;

    TRY
    {
        if (pthread_rwlock_wrlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_wrlock");
        }

	iterFuncArg.display = display;
	iterFuncArg.func    = func;
	iterFuncArg.arg     = arg;

	if (gvForeachProcessStateItem(contextIterFunc,
				      &iterFuncArg) == -1)
	{
	    THROW(e1, "gvForeachProcessStateItem");
	}

        if (pthread_rwlock_unlock(&processStateLock) != 0)
        {
            THROW(e1, "pthread_rwlock_unlock");
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
        pthread_rwlock_unlock(&processStateLock);
        return -1;
    }

    return 0;
}

int
gvPutEglContextState(EGLDisplay        display,
		     EGLContext        context,
		     GVcontextstateptr state)
{
    TRY
    {
        if (pthread_rwlock_wrlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_wrlock");
        }

	if (gvPutProcessStateItem((size_t) context, state) == -1)
	{ 
	    THROW(e1, "gvPutProcessStateItem");
	}

        if (pthread_rwlock_unlock(&processStateLock) != 0)
        {
            THROW(e1, "pthread_rwlock_unlock");
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
        pthread_rwlock_unlock(&processStateLock);
        return -1;
    }

    return 0;
}

int
gvIsMarkedCurrent(EGLDisplay display,
                  EGLContext context)
{
    int               markedCurrent;
    GVcontextstateptr state;

    TRY
    {
        if (pthread_rwlock_rdlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_rdlock");
        }

	if ((state
	     = (GVcontextstateptr) gvGetProcessStateItem(
		 (size_t) context)) == NULL)
	{
	    THROW(e1, "gvGetProcessStateItem");
	}

	markedCurrent = state->markedCurrent;

        if (pthread_rwlock_unlock(&processStateLock) != 0)
        {
            THROW(e1, "pthread_rwlock_unlock");
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
        pthread_rwlock_unlock(&processStateLock);
        return -1;
    }

    return markedCurrent;
}

int gvSetMarkedCurrent(EGLDisplay display,
		       EGLContext context,
		       int        markedCurrent)
{
    GVcontextstateptr state;

    TRY
    {
        if (pthread_rwlock_wrlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_wrlock");
        }

	if ((state
	     = (GVcontextstateptr) gvGetProcessStateItem(
		 (size_t) context)) == NULL)
	{
	    THROW(e1, "gvGetProcessStateItem");
	}

        state->markedCurrent = markedCurrent;

        if (pthread_rwlock_unlock(&processStateLock) != 0)
        {
            THROW(e1, "pthread_rwlock_unlock");
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
        pthread_rwlock_unlock(&processStateLock);
        return -1;
    }

    return 0;
}

int
gvIsMarkedDestroyed(EGLDisplay display,
                    EGLContext context)
{
    int               markedDestroyed;
    GVcontextstateptr state;

    TRY
    {
        if (pthread_rwlock_rdlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_rdlock");
        }

	if ((state
	     = (GVcontextstateptr) gvGetProcessStateItem(
		 (size_t) context)) == NULL)
	{
	    THROW(e1, "gvGetProcessStateItem");
	}

	markedDestroyed = state->markedDestroyed;

        if (pthread_rwlock_unlock(&processStateLock) != 0)
        {
            THROW(e1, "pthread_rwlock_unlock");
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
        pthread_rwlock_unlock(&processStateLock);
        return -1;
    }

    return markedDestroyed;
}

int
gvSetMarkedDestroyed(EGLDisplay display,
		     EGLContext context,
		     int        markedDestroyed)
{
    GVcontextstateptr state;

    TRY
    {
        if (pthread_rwlock_wrlock(&processStateLock) != 0)
        {
            THROW(e0, "pthread_rwlock_wrlock");
        }

	if ((state
	     = (GVcontextstateptr) gvGetProcessStateItem(
		 (size_t) context)) == NULL)
	{
	    THROW(e1, "gvGetProcessStateItem");
	}

        state->markedDestroyed = markedDestroyed;

        if (pthread_rwlock_unlock(&processStateLock) != 0)
        {
            THROW(e1, "pthread_rwlock_unlock");
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
        pthread_rwlock_unlock(&processStateLock);
        return -1;
    }

    return 0;
}

int
gvSetAllMarkedDestroyed(EGLDisplay display,
			int        markedDestroyed)
{
    TRY
    {
	if (gvForeachEglContextState(display,
				     setAllMarkedDestroyedIterFunc,
				     &markedDestroyed) == -1)
	{
	    THROW(e0, "gvForeachEglContextState");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

/* ***************************************************************************
 * Thread display, context, transport
 */

EGLDisplay
gvGetCurrentThreadDisplay()
{
    EGLDisplay display;

    initIfNotDoneAlready();

    TRY
    {
	if ((display
	     = (EGLDisplay) gvGetThreadStateItem(CURRENT_DISPLAY_KEY)) == NULL)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return display;
}

int
gvSetCurrentThreadDisplay(EGLDisplay display)
{
    initIfNotDoneAlready();

    TRY
    {
	if (gvPutThreadStateItem(CURRENT_DISPLAY_KEY, display) == -1)
	{
	    THROW(e0, "gvPutThreadStateItem");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

EGLContext
gvGetCurrentThreadContext()
{
    EGLContext context;

    initIfNotDoneAlready();

    TRY
    {
	if ((context
	     = (EGLContext) gvGetThreadStateItem(CURRENT_CONTEXT_KEY)) == NULL)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return context;
}

int
gvSetCurrentThreadContext(EGLContext context)
{
    initIfNotDoneAlready();

    TRY
    {
	if (gvPutThreadStateItem(CURRENT_CONTEXT_KEY, context) == -1)
	{
	    THROW(e0, "gvPutThreadStateItem");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

GVtransportptr
gvGetCurrentThreadTransport(void)
{
    GVtransportptr transport;

    initIfNotDoneAlready();

    TRY
    {
	if ((transport
	     = (GVtransportptr) gvGetThreadStateItem(
		 CURRENT_TRANSPORT_KEY)) == NULL)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return transport;
}

int
gvSetCurrentThreadTransport(GVtransportptr transport)
{
    initIfNotDoneAlready();

    TRY
    {
	if (gvPutThreadStateItem(CURRENT_TRANSPORT_KEY, transport) == -1)
	{
	    THROW(e0, "gvPutThreadStateItem");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

/* ***************************************************************************
 * Vertex attribs
 */

int
gvDelVertexAttrib(GLuint index)
{
    GVvertexattribptr attribs;

    TRY
    {
	if ((attribs
	     = (GVvertexattribptr) gvGetThreadStateItem(
		 VERTEX_ATTRIB_KEY)) == NULL)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}	

	memset(&attribs[index], 0, sizeof(struct GVvertexattrib));
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

GVvertexattribptr
gvGetVertexAttrib(GLuint index)
{
    GVvertexattribptr attribs;

    TRY
    {
	if ((attribs
	     = (GVvertexattribptr) gvGetThreadStateItem(
		 VERTEX_ATTRIB_KEY)) == NULL)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return &attribs[index];
}

int
gvPutVertexAttrib(GLuint            index,
		  GVvertexattribptr vertexAttrib)
{
    GVvertexattribptr attribs;

    TRY
    {
	if ((attribs
	     = (GVvertexattribptr) gvGetThreadStateItem(
		 VERTEX_ATTRIB_KEY)) == NULL)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}

	memcpy(&attribs[index], vertexAttrib, sizeof(struct GVvertexattrib));
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

static int
getNumEnabledVertexAttribs()
{
    int *num;

    TRY
    { 
	if ((num = (int *)gvGetThreadStateItem(
		 NUM_ENABLED_VERTEX_ATTRIBS_KEY)) == NULL)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return *num;
}

static int
setNumEnabledVertexAttribs(int num)
{
    int *tempNum;

    TRY
    { 
	if ((tempNum = (int *)gvGetThreadStateItem(
		 NUM_ENABLED_VERTEX_ATTRIBS_KEY)) == NULL)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}
    
	*tempNum = num;
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

static int
incNumEnabledVertexAttribs()
{
    int num;

    TRY
    { 
	if ((num = getNumEnabledVertexAttribs()) == -1)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}

	if (setNumEnabledVertexAttribs(num + 1) == -1)
	{
	    THROW(e0, "setNumEnabledVertexAttribs");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

static int
decNumEnabledVertexAttribs()
{
    int num;

    TRY
    { 
	if ((num = getNumEnabledVertexAttribs()) == -1)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}

	if (num > 0)
	{
	    if (setNumEnabledVertexAttribs(num - 1) == -1)
	    {
		THROW(e0, "setNumEnabledVertexAttribs");
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
gvEnableVertexAttrib(GLuint index)
{
    GVvertexattribptr attribs;

    TRY
    {
	if ((attribs
	     = (GVvertexattribptr) gvGetThreadStateItem(
		 VERTEX_ATTRIB_KEY)) == NULL)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}

	if (!attribs[index].enabled)
	{
	    attribs[index].enabled = 1;

	    if (incNumEnabledVertexAttribs() == -1)
	    {
		THROW(e0, "incNumEnabledVertexAttribs");
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
gvDisableVertexAttrib(GLuint index)
{
    GVvertexattribptr attribs;

    TRY
    {
	if ((attribs
	     = (GVvertexattribptr) gvGetThreadStateItem(
		 VERTEX_ATTRIB_KEY)) == NULL)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}

	if (attribs[index].enabled)
	{
	    attribs[index].enabled = 0;

	    if (decNumEnabledVertexAttribs() == -1)
	    {
		THROW(e0, "decNumEnabledVertexAttribs");
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
gvGetEnabledVertexAttribs(GVvertexattribptr *attribs,
			  int               *numAttribs)
{
    GVvertexattribptr tempAttribs;
    int               tempNumAttribs;

    TRY
    { 
	if ((tempAttribs
	     = (GVvertexattribptr) gvGetThreadStateItem(
		 VERTEX_ATTRIB_KEY)) == NULL)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}

	if ((tempNumAttribs = getNumEnabledVertexAttribs()) == -1)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    *attribs    = tempAttribs;
    *numAttribs = tempNumAttribs;

    return 0;
}

/* ***************************************************************************
 * Vertex buffers
 */

int
gvSetBufferBound(GLenum target,
		 GLuint buffer)
{
    struct VertexBuffer **buffers, *tempBuffer;

    TRY
    { 
	if ((buffers
	     = (struct VertexBuffer **)gvGetThreadStateItem(
		 VERTEX_BUFFER_KEY)) == NULL)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}

	HASH_FIND_INT(buffers[target % 2], &buffer, tempBuffer);
	if (tempBuffer == NULL)
	{
	    tempBuffer = malloc(sizeof(struct VertexBuffer));
	    tempBuffer->id = buffer;
	    HASH_ADD_INT(buffers[target % 2], id, tempBuffer);
	}

    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvIsAnyBufferBound()
{
    struct VertexBuffer **buffers;

    TRY
    { 
	if ((buffers
	     = (struct VertexBuffer **)gvGetThreadStateItem(
		 VERTEX_BUFFER_KEY)) == NULL)
	{
	    THROW(e0, "gvGetThreadStateItem");
	}

	return HASH_COUNT(buffers[0]) > 0 || HASH_COUNT(buffers[1]) > 0;
    }
    CATCH (e0)
    {
	return -1;
    }
}

