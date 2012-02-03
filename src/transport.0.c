/*! ***************************************************************************
 * \file    transport.0.c
 * \brief   
 * 
 * \date    December 20, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <errno.h>
#include <string.h>
#include <vrb.h>
#include <sys/mman.h>

#include "error.h"
#include "shared_memory.h"
#include "sleep.h"
#include "transport.h"

/* ****************************************************************************
 * Buffer
 */

struct Buffer {

    /* App. address space */
    struct GVbuffer  public;


    struct InAppBufferPart {
	void        *tail;
    }  inAppPart;
    

    /* Shared memory */
    struct InShmBufferPart {
	VRB              head;

	GVlock           lock;

	struct GVcondvar readCondVar;
	struct GVcondvar writeCondVar;
    } *inShmPart;

};

#define castBuffer(buffer) \
    ((struct Buffer *)buffer)

/* ****************************************************************************
 * Transport
 */

#define STATE_INITIALIZED   0x1
#define STATE_UNINITIALIZED 0x0

struct Transport {

    /* App. address space */
    struct GVtransport         public;

    /* Shared memory */
    struct InShmTransportPart {
	int                    state;

	struct InShmBufferPart callBuffer;
	struct InShmBufferPart returnBuffer;
    } *inShmPart;

};

#define castTransport(transport) \
    ((struct Transport *)transport)

/* ****************************************************************************
 * File / Memory Layout 
 */
 
/* 1. physical file in tempfs:
 * ---------------------------
 * 
 *    ____________________ length ____________________
 *   /                                                \
 *   --------------------------------------------------  
 *   |         |                                      |
 *   --------------------------------------------------
 *        |                        |                   
 *     transport head           transport tail 
 *
 *
 * 1.1 transport head:
 * -------------------
 *
 *    ____________ 1 * system page size ______________
 *   /                                                \
 *     call buffer head (+ lock, cond)  empty
 *            |                           |
 *   --------------------------------------------------
 *   |   |         |         |                        |
 *   --------------------------------------------------
 *     |                |
 *   state     return buffer head (+ locks)
 *
 *
 * 1.2 transport tail:
 * -------------------
 *
 *    _________ length - system page size ____________
 *   /                                                \
 *   --------------------------------------------------
 *   |                       |                        |
 *   --------------------------------------------------
 *              |                        |
 *      call buffer tail       return buffer tail
 *
 *
 * 2. logical mmaped file:
 * -----------------------
 *
 * The buffer tails are mainShmed twice to a consecutive logical memory region,
 * so the logical length is: transport head size + 2 * transport tail size
 *
 *    __________ 2 * transport tail size _____________
 *   /                                                \
 *   --------------------------------------------------
 *   |           |           |           |            |
 *   --------------------------------------------------
 *   \______________________/\________________________/
 *               |                       |
 *     2 * call buffer tail    2 * return buffer tail
 */

/* TODO not portable, use getSystemPageSize() */
static int systemPageSize = 4096;

#define TRANSPORT_HEAD_SIZE                       systemPageSize
#define TRANSPORT_TAIL_SIZE(length)               (length - systemPageSize)
 
#define BUFFER_HEAD_SIZE                          sizeof(VRB)
#define BUFFER_TAIL_SIZE(length)                  (TRANSPORT_TAIL_SIZE(length) >> 1)

/* Physical offsets (temp file) */

#define FILE_TRANSPORT_HEAD_OFFSET                 0
#define FILE_TRANSPORT_TAIL_OFFSET                 systemPageSize

#define FILE_CALL_BUFFER_TAIL_OFFSET               FILE_TRANSPORT_TAIL_OFFSET
#define FILE_RETURN_BUFFER_TAIL_OFFSET(length)     FILE_CALL_BUFFER_TAIL_OFFSET       \
                                                   + BUFFER_TAIL_SIZE(length)
/* Logical offsets (mmap) */

#define MMAP_TRANSPORT_HEAD_OFFSET                 0
#define MMAP_TRANSPORT_TAIL_OFFSET                 systemPageSize

#define MMAP_CALL_BUFFER_TAIL_1ST_OFFSET           MMAP_TRANSPORT_TAIL_OFFSET
#define MMAP_CALL_BUFFER_TAIL_2ND_OFFSET(length)   MMAP_TRANSPORT_TAIL_OFFSET         \
                                                   + BUFFER_TAIL_SIZE(length)

#define MMAP_RETURN_BUFFER_TAIL_1ST_OFFSET(length) MMAP_TRANSPORT_TAIL_OFFSET         \
                                                   + TRANSPORT_TAIL_SIZE(length)
#define MMAP_RETURN_BUFFER_TAIL_2ND_OFFSET(length) MMAP_TRANSPORT_TAIL_OFFSET         \
                                                   + TRANSPORT_TAIL_SIZE(length)      \
                                                   + BUFFER_TAIL_SIZE(length)

/* ****************************************************************************
 */

static int
initSyncPrimitives(struct Buffer *callBuffer, struct Buffer *returnBuffer)
{
    GVlockptr    callLock;
    GVcondvarptr callReadCondVar;
    GVcondvarptr callWriteCondVar;
    GVlockptr    returnLock;
    GVcondvarptr returnReadCondVar;
    GVcondvarptr returnWriteCondVar;

    TRY
    {
	if ((callLock = gvCreateLock()) == NULL)
	{
	    THROW(e0, "gvCreateLock");
	}

	if ((callReadCondVar = gvCreateCondVar()) == NULL)
	{
	    THROW(e0, "gvCreateCond");
	}

	if ((callWriteCondVar = gvCreateCondVar()) == NULL)
	{
	    THROW(e0, "gvCreateCond");
	}

	if ((returnLock = gvCreateLock()) == NULL)
	{
	    THROW(e0, "gvCreateLock");
	}

	if ((returnReadCondVar = gvCreateCondVar()) == NULL)
	{
	    THROW(e0, "gvCreateCond");
	}

	if ((returnWriteCondVar = gvCreateCondVar()) == NULL)
	{
	    THROW(e0, "gvCreateCond");
	}

        memcpy(&callBuffer->inShmPart->lock, callLock, sizeof(GVlock));
        memcpy(&callBuffer->inShmPart->readCondVar, callReadCondVar, sizeof(struct GVcondvar));
        memcpy(&callBuffer->inShmPart->writeCondVar, callWriteCondVar, sizeof(struct GVcondvar));
        memcpy(&returnBuffer->inShmPart->lock, returnLock, sizeof(GVlock));
        memcpy(&returnBuffer->inShmPart->readCondVar, returnReadCondVar, sizeof(struct GVcondvar));
        memcpy(&returnBuffer->inShmPart->writeCondVar, returnWriteCondVar, sizeof(struct GVcondvar));
	
	free(callLock);
	free(callReadCondVar);
	free(callWriteCondVar);
	free(returnLock);
	free(returnReadCondVar);
	free(returnWriteCondVar);
    }
    CATCH (e0)
    {
	return -1;
    }
 
    return 0;
}

/** ***************************************************************************
 */

GVtransportptr
gvCreateTransport(GVshmptr shm, size_t offset, size_t length)
{
   
    void             *base;
    struct Transport *transport;
    struct Buffer    *callBuffer;
    struct Buffer    *returnBuffer;

    TRY
    {
	/*
	 * Constraint:
	 *
	 *   transport tail size / 2 = n * system page size, n is positive
	 *
	 */
	if (!(length > 0 && BUFFER_TAIL_SIZE(length) % systemPageSize == 0))
	{
	    errno = EINVAL;
	    THROW(e0, "invalid length");
	}

	if ((base = mmap(NULL, length + TRANSPORT_TAIL_SIZE(length), PROT_NONE,
			 MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
	{
	    THROW(e0, "mmap");
	}

	if (gvAttachShm(base   + MMAP_TRANSPORT_HEAD_OFFSET,
			shm,
			offset + FILE_TRANSPORT_HEAD_OFFSET,
			TRANSPORT_HEAD_SIZE) == -1)
	{
	    THROW(e0, "gvAttachShm");
	}

	if (gvAttachShm(base   + MMAP_CALL_BUFFER_TAIL_1ST_OFFSET,
			shm,
			offset + FILE_CALL_BUFFER_TAIL_OFFSET,
			BUFFER_TAIL_SIZE(length)) == -1)
	{
	    THROW(e0, "gvAttachShm");
	}

	if (gvAttachShm(base   + MMAP_CALL_BUFFER_TAIL_2ND_OFFSET(length),
			shm,
			offset + FILE_CALL_BUFFER_TAIL_OFFSET,
			BUFFER_TAIL_SIZE(length)) == -1)
	{
	    THROW(e0, "gvAttachShm");
	}

	if (gvAttachShm(base   + MMAP_RETURN_BUFFER_TAIL_1ST_OFFSET(length),
			shm,
			offset + FILE_RETURN_BUFFER_TAIL_OFFSET(length),
			BUFFER_TAIL_SIZE(length)) == -1)
	{
	    THROW(e0, "gvAttachShm");
	}

	if (gvAttachShm(base   + MMAP_RETURN_BUFFER_TAIL_2ND_OFFSET(length),
			shm,
			offset + FILE_RETURN_BUFFER_TAIL_OFFSET(length),
			BUFFER_TAIL_SIZE(length)) == -1)
	{
	    THROW(e0, "gvAttachShm");
	}

	if ((transport = malloc(sizeof(struct Transport))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	if ((callBuffer = malloc(sizeof(struct Buffer))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	if ((returnBuffer = malloc(sizeof(struct Buffer))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	transport->inShmPart = base;

	callBuffer->inShmPart = &transport->inShmPart->callBuffer;
	callBuffer->inAppPart.tail = base + MMAP_CALL_BUFFER_TAIL_1ST_OFFSET;

	returnBuffer->inShmPart = &transport->inShmPart->returnBuffer;
	returnBuffer->inAppPart.tail = base + MMAP_RETURN_BUFFER_TAIL_1ST_OFFSET(length);

	if (transport->inShmPart->state == STATE_UNINITIALIZED)
	{
	    vrb_p callBufferHead   = &callBuffer->inShmPart->head;
	    vrb_p returnBufferHead = &returnBuffer->inShmPart->head;

	    transport->inShmPart->state = STATE_INITIALIZED;

	    callBufferHead->lower_ptr = 0;
	    callBufferHead->upper_ptr = (void *)BUFFER_TAIL_SIZE(length);
	    callBufferHead->first_ptr = 0;
	    callBufferHead->last_ptr  = 0;
	    callBufferHead->mem_ptr   = 0;
	    callBufferHead->buf_size  = BUFFER_TAIL_SIZE(length);
	    vrb_set_mmap(callBufferHead);

	    returnBufferHead->lower_ptr = 0;
	    returnBufferHead->upper_ptr = (void *)BUFFER_TAIL_SIZE(length);
	    returnBufferHead->first_ptr = 0;
	    returnBufferHead->last_ptr  = 0;
	    returnBufferHead->mem_ptr   = 0;
	    returnBufferHead->buf_size  = BUFFER_TAIL_SIZE(length);
	    vrb_set_mmap(returnBufferHead);

	    initSyncPrimitives(callBuffer, returnBuffer);
	}
	
 	transport->public.shm    = shm;
	transport->public.offset = offset;
	transport->public.length = length;

	transport->public.callBuffer   = (GVbufferptr) callBuffer;
	transport->public.returnBuffer = (GVbufferptr) returnBuffer;

	transport->public.callBuffer->lock   = &callBuffer->inShmPart->lock;
	transport->public.returnBuffer->lock = &returnBuffer->inShmPart->lock;
    }
    CATCH (e0)
    {
	return NULL;
    }

    return (GVtransportptr) transport;
}

void
*gvDataPtr(GVbufferptr buffer)
{
    size_t offset;

    offset = (size_t) vrb_data_ptr(&castBuffer(buffer)->inShmPart->head);

    return castBuffer(buffer)->inAppPart.tail + offset;
}

size_t
gvDataLength(GVbufferptr buffer)
{
    return vrb_data_len(&castBuffer(buffer)->inShmPart->head);
}

int
gvTake(GVbufferptr buffer, size_t length)
{
    TRY
    {
	if (vrb_take(&castBuffer(buffer)->inShmPart->head, length) == -1)
	{
	    THROW(e0, "vrb_take");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvRead(GVbufferptr buffer, void *addr, size_t restLength)
{
    GVcondvarptr  readCondVar  = &castBuffer(buffer)->inShmPart->readCondVar;
    GVcondvarptr  writeCondVar = &castBuffer(buffer)->inShmPart->writeCondVar;
    vrb_p         head         = &castBuffer(buffer)->inShmPart->head;
    void         *tail         = castBuffer(buffer)->inAppPart.tail;

    size_t        offset;
    size_t        dataLength;

    int           done         = 0;

    TRY 
    {
	while (1) 
	{
	    offset = (size_t) vrb_data_ptr(head);

	    if (gvAcquire(readCondVar->lock) == -1)
	    {
		THROW(e0, "gvAcquire");
	    }

	    while ((dataLength = vrb_data_len(head)) == 0)
	    {
		if (gvWait(readCondVar) == -1)
		{
		    THROW(e1, "gvWait");
		}
	    }

	    if (gvRelease(readCondVar->lock) == -1)
	    {
		THROW(e0, "gvRelease");
	    }

	    if (dataLength >= restLength)
	    {
		memcpy(addr, tail + offset, restLength);

		if (vrb_take(head, restLength) == -1)
		{
		    THROW(e1, "vrb_take");
		}

		done = 1;
	    }
	    else
	    {
		memcpy(addr, tail + offset, dataLength);
		
		if (vrb_take(head, dataLength) == -1)
		{
		    THROW(e1, "vrb_take");
		}

		addr       = addr + dataLength;
		restLength = restLength - dataLength;
	    }

	    if (gvAcquire(writeCondVar->lock) == -1)
	    {
		THROW(e1, "gvRelease");
	    }

	    if (gvNotify(writeCondVar) == -1)
	    {
		THROW(e1, "gvNotify");
	    }

	    if (gvRelease(writeCondVar->lock) == -1)
	    {
		THROW(e1, "gvRelease");
	    }

	    if (done)
	    {
		break;
	    }
	}

    }
    CATCH (e0)
    {
	/* Try to unlock. At this point we can't do anything if unlocking
	 * fails, so just ignore errors.
	 */
	gvRelease(readCondVar->lock);

	return -1;
    }
    CATCH (e1)
    {
	return -1;
    }

    return 0;
}

void
*gvSpacePtr(GVbufferptr buffer)
{
    size_t offset;

    offset = (size_t) vrb_space_ptr(&castBuffer(buffer)->inShmPart->head);

    return  castBuffer(buffer)->inAppPart.tail + offset;
}

size_t
gvSpaceLength(GVbufferptr buffer)
{
    return vrb_space_len(&castBuffer(buffer)->inShmPart->head);
}

int
gvGive(GVbufferptr buffer, size_t length)
{
    TRY
    {
	if (vrb_give(&castBuffer(buffer)->inShmPart->head, length) == -1)
	{
	    THROW(e0, "vrb_give");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvWrite(GVbufferptr buffer, const void *addr, size_t restLength)
{
    GVcondvarptr  readCondVar  = &castBuffer(buffer)->inShmPart->readCondVar;
    GVcondvarptr  writeCondVar = &castBuffer(buffer)->inShmPart->writeCondVar;
    vrb_p         head         = &castBuffer(buffer)->inShmPart->head;
    void         *tail         = castBuffer(buffer)->inAppPart.tail;

    size_t        offset;
    size_t        spaceLength;

    int           done         = 0;

    TRY
    {
	while (1) 
	{
	    offset = (size_t) vrb_space_ptr(head);


	    if (gvAcquire(writeCondVar->lock) == -1)
	    {
		THROW(e0, "gvAcquire");
	    }

	    while ((spaceLength = vrb_space_len(head)) == 0)
	    {
		if (gvWait(writeCondVar) == -1)
		{
		    THROW(e1, "gvWait");
		}
	    }

	    if (gvRelease(writeCondVar->lock) == -1)
	    {
		THROW(e0, "gvAcquire");
	    }

	    if (spaceLength >= restLength)
	    {
		memcpy(tail + offset, addr, restLength);
	
		if (vrb_give(head, restLength) == -1)
		{
		    THROW(e1, "vrb_give");
		}

		done = 1;
	    }
	    else if (spaceLength > 0)
	    {
		memcpy(tail + offset, addr, spaceLength);

		if (vrb_give(head, spaceLength) == -1)
		{
		    THROW(e1, "vrb_give");
		}

		addr       = addr + spaceLength;
		restLength = restLength - spaceLength;
	    }

	    if (gvAcquire(readCondVar->lock) == -1)
	    {
		THROW(e0, "gvRelease");
	    }

	    if (gvNotify(readCondVar) == -1)
	    {
		THROW(e1, "gvNotify");
	    }

	    if (gvRelease(readCondVar->lock) == -1)
	    {
		THROW(e0, "gvRelease");
	    }

	    if (done)
	    {
		break;
	    }
	}
    }
    CATCH (e0)
    {
	/* Try to unlock. At this point we can't do anything if unlocking
	 * fails, so just ignore errors.
	 */
	gvRelease(writeCondVar->lock);

	return -1;
    }
    CATCH (e1)
    {
	return -1;
    }

    return 0;
}

int
gvDestroyTransport(GVtransportptr transport)
{
    size_t length = transport->length;

    TRY
    {
	if (munmap(castTransport(transport)->inShmPart,
		   length + TRANSPORT_TAIL_SIZE(length)))
	{
	    THROW(e0, "munmap");
	}

	free(transport->callBuffer);
	free(transport->returnBuffer);
	free(transport);
    }
    CATCH (e0)
    {
	return -1;
    }

   return 0;
}
