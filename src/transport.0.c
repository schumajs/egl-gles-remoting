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
#include <stdio.h>
#include <string.h>
#include <vrb.h>
#include <sys/mman.h>

#include "shared_memory.h"
#include "sleep.h"
#include "transport.h"

/* ****************************************************************************
 * Buffer
 */

struct Buffer {

    /* Transient part */
    struct GVTRPbuffer    public;
    void                 *tail;

    /* Persistent part */
    struct PersistentBufferPart {
	VRB               head;

	GVLCKlock  clientLock;
	GVLCKlock  serverLock;
    } *inShm;

};

#define castBuffer(buffer) \
    ((struct Buffer *)buffer)

/* ****************************************************************************
 * Transport
 */

#define STATE_INITIALIZED   0x1
#define STATE_UNINITIALIZED 0x0

struct Transport {

    /* Transient part */
    struct GVTRPtransport public;

    /* Persistent part */
    struct PersistentTransportPart {
	int state;

	struct PersistentBufferPart callBuffer;
	struct PersistentBufferPart returnBuffer;
    } *inShm;

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
 *     call buffer head (+ locks)       empty
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
 * The buffer tails are mainShmed twice to a consecutive logical memory region, so
 * the logical length is: transport head size + 2 * transport tail size
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

#define initLocks(callBuffer, returnBuffer)				       \
    do {								       \
        GVLCKlockptr tempLock;						       \
									       \
	if (gvlckCreate(&tempLock) == -1)				       \
	{								       \
	    perror("gvlckCreate");					       \
	    return -1;							       \
	}								       \
									       \
	memcpy(&callBuffer->inShm->clientLock, tempLock, sizeof(GVLCKlock));   \
	memcpy(&callBuffer->inShm->serverLock, tempLock, sizeof(GVLCKlock));   \
									       \
	memcpy(&returnBuffer->inShm->clientLock, tempLock, sizeof(GVLCKlock)); \
	memcpy(&returnBuffer->inShm->serverLock, tempLock, sizeof(GVLCKlock)); \
									       \
        if (gvlckDestroy(tempLock) == -1)				       \
        {								       \
	    perror("gvlckDestroy");					       \
	    return -1;							       \
        }								       \
    } while(0)

/** ***************************************************************************
 */

int
gvtrpCreate(GVTRPtransportptr *newTransport, 
	    GVSHMshmptr shm, size_t offset, size_t length)
{
   
    void             *base;
    struct Transport *transport;
    struct Buffer    *callBuffer;
    struct Buffer    *returnBuffer;

    /*
     * Constraint:
     *
     *   transport tail size / 2 = n * system page size, n is positive
     *
     */
    if (!(length > 0 && BUFFER_TAIL_SIZE(length) % systemPageSize == 0))
    {
	errno = EINVAL;
	return -1;
    }

    if ((base = mmap(NULL, length + TRANSPORT_TAIL_SIZE(length), PROT_NONE,
		     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
    {
	perror("mmap");
	return -1;
    }

    if (gvshmAttach(base   + MMAP_TRANSPORT_HEAD_OFFSET,
		    shm,
		    offset + FILE_TRANSPORT_HEAD_OFFSET,
		    TRANSPORT_HEAD_SIZE) == -1)
    {
	perror("gvshmAttach");
	return -1;
    }

    if (gvshmAttach(base   + MMAP_CALL_BUFFER_TAIL_1ST_OFFSET,
		    shm,
		    offset + FILE_CALL_BUFFER_TAIL_OFFSET,
		    BUFFER_TAIL_SIZE(length)) == -1)
    {
	perror("gvshmAttach");
	return -1;
    }

    if (gvshmAttach(base   + MMAP_CALL_BUFFER_TAIL_2ND_OFFSET(length),
		    shm,
		    offset + FILE_CALL_BUFFER_TAIL_OFFSET,
		    BUFFER_TAIL_SIZE(length)) == -1)
    {
	perror("gvshmAttach");
	return -1;
    }

    if (gvshmAttach(base   + MMAP_RETURN_BUFFER_TAIL_1ST_OFFSET(length),
		    shm,
		    offset + FILE_RETURN_BUFFER_TAIL_OFFSET(length),
		    BUFFER_TAIL_SIZE(length)) == -1)
    {
	perror("gvshmAttach");
	return -1;
    }

    if (gvshmAttach(base   + MMAP_RETURN_BUFFER_TAIL_2ND_OFFSET(length),
		    shm,
		    offset + FILE_RETURN_BUFFER_TAIL_OFFSET(length),
		    BUFFER_TAIL_SIZE(length)) == -1)
    {
	perror("gvshmAttach");
	return -1;
    }

    transport = malloc(sizeof(struct Transport));
    transport->inShm = base;

    callBuffer = malloc(sizeof(struct Buffer));
    callBuffer->inShm = &transport->inShm->callBuffer;

    callBuffer->tail = base + MMAP_CALL_BUFFER_TAIL_1ST_OFFSET;

    returnBuffer = malloc(sizeof(struct Buffer));
    returnBuffer->inShm = &transport->inShm->callBuffer;

    returnBuffer->tail = base + MMAP_RETURN_BUFFER_TAIL_1ST_OFFSET(length);

    if (transport->inShm->state == STATE_UNINITIALIZED)
    {
	vrb_p callBufferHead   = &callBuffer->inShm->head;
	vrb_p returnBufferHead = &returnBuffer->inShm->head;

	transport->inShm->state = STATE_INITIALIZED;

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

	initLocks(callBuffer, returnBuffer);
    }

    *newTransport = (GVTRPtransportptr) transport;

    (*newTransport)->shm    = shm;
    (*newTransport)->offset = offset;
    (*newTransport)->length = length;

    (*newTransport)->callBuffer   = (GVTRPbufferptr) callBuffer;
    (*newTransport)->returnBuffer = (GVTRPbufferptr) returnBuffer;

    (*newTransport)->callBuffer->clientLock  = &callBuffer->inShm->clientLock;
    (*newTransport)->callBuffer->serverLock  = &callBuffer->inShm->serverLock;

    (*newTransport)->returnBuffer->clientLock = &returnBuffer->inShm->clientLock;
    (*newTransport)->returnBuffer->serverLock = &returnBuffer->inShm->serverLock;

    return 0;
}

int
gvtrpDataPtr(GVTRPbufferptr buffer, void **dataPtr)
{
    size_t offset;

    offset = (size_t) vrb_data_ptr(&castBuffer(buffer)->inShm->head);
    *dataPtr = castBuffer(buffer)->tail + offset;

    return 0;
}

int
gvtrpDataLength(GVTRPbufferptr buffer, size_t *length)
{
    *length = vrb_data_len(&castBuffer(buffer)->inShm->head);
    return 0;
}

int
gvtrpTake(GVTRPbufferptr buffer, size_t length)
{
    vrb_take(&castBuffer(buffer)->inShm->head, length);
    return 0;
}

int
gvtrpRead(GVTRPbufferptr buffer, void *addr, size_t restLength)
{
    vrb_p   head = &castBuffer(buffer)->inShm->head;
    void   *tail = castBuffer(buffer)->tail;

    size_t  offset;
    size_t  dataLength;

    while (1) 
    {
	/* TODO synchronization? */
	offset     = (size_t) vrb_data_ptr(head);
	dataLength = vrb_data_len(head);

	if (dataLength >= restLength)
	{
	    memcpy(addr, tail + offset, restLength);
	    vrb_take(head, restLength);

	    break;
	}
	else if (dataLength > 0)
	{
	    memcpy(addr, tail + offset, dataLength);
	    vrb_take(head, dataLength);

	    addr       = addr + dataLength;
	    restLength = restLength - dataLength;
	}

	gvslpSleep(0, 1000);
    }

    return 0;
}

int
gvtrpSpacePtr(GVTRPbufferptr buffer, void **spacePtr)
{
    size_t offset;

    offset = (size_t) vrb_data_ptr(&castBuffer(buffer)->inShm->head);
    *spacePtr = castBuffer(buffer)->tail + offset;

    return 0;
}

int
gvtrpSpaceLength(GVTRPbufferptr buffer, size_t *length)
{
    *length = vrb_space_len(&castBuffer(buffer)->inShm->head);
    return 0;
}

int
gvtrpGive(GVTRPbufferptr buffer, size_t length)
{
    vrb_give(&castBuffer(buffer)->inShm->head, length);
    return 0;
}

int
gvtrpWrite(GVTRPbufferptr buffer, const void *addr, size_t restLength)
{
    vrb_p   head = &castBuffer(buffer)->inShm->head;
    void   *tail = castBuffer(buffer)->tail;

    size_t  offset;
    size_t  spaceLength;

    while (1) 
    {
	/* TODO synchronization? */
	offset     = (size_t) vrb_space_ptr(head);
	spaceLength = vrb_space_len(head);

	if (spaceLength >= restLength)
	{
	    memcpy(tail + offset, addr, restLength);
	    vrb_give(head, restLength);

	    break;
	}
	else if (spaceLength > 0)
	{
	    memcpy(tail + offset, addr, spaceLength);
	    vrb_give(head, spaceLength);

	    addr       = addr + spaceLength;
	    restLength = restLength - spaceLength;
	}

	gvslpSleep(0, 1000);
    }

    return 0;
}

int
gvtrpDestroy(GVTRPtransportptr transport)
{
    size_t length = transport->length;

    if (munmap(castTransport(transport)->inShm,
	       length + TRANSPORT_TAIL_SIZE(length)))
    {
	perror("munmap");
	return -1;
    }

   free(transport->callBuffer);
   free(transport->returnBuffer);
   free(transport);

   return 0;
}
