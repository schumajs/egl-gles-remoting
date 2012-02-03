/*! ***************************************************************************
 * \file    shared_memory_transport.0.c
 * \brief   
 * 
 * \date    December 20, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <vrb.h>
#include <sys/mman.h>

#include "cond_var.h"
#include "error.h"
#include "shared_memory.h"
#include "shared_memory_transport.h"

/* ****************************************************************************
 * GVstream implementation
 */

struct Stream {

    /* App. address space */
    struct GVstream  base;

    struct AppPart {
	void        *vrbTail;
    } app;
    

    /* Shared memory */
    struct StreamShmPart {
	VRB       vrbHead;

	GVlock    exclusiveAccess;

	GVcondvar dataAvailable;
	GVlock    dataAvailableAccess;

	GVcondvar spaceAvailable;
	GVlock    spaceAvailableAccess;
    } *shm;

};

#define castStream(stream) \
    ((struct Stream *)stream)

/* ****************************************************************************
 * GVtransport implementation
 */

struct Transport {

    /* App. address space */
    struct GVshmtransport base;

    /* Shared memory */
    struct TransportShmPart {
	int                  state;

	struct StreamShmPart is;
	struct StreamShmPart os;
    } *shm;

};

#define castTransport(transport) \
    ((struct Transport *)transport)

/** ***************************************************************************
 * Operations
 */

static int
readFunc(GVstreamptr  stream,
	 void        *toAddr,
	 size_t       restLength)
{
    GVcondvarptr  dataAvailable        = &castStream(stream)->shm->dataAvailable;
    GVlockptr     dataAvailableAccess  = &castStream(stream)->shm->dataAvailableAccess;

    GVcondvarptr  spaceAvailable       = &castStream(stream)->shm->spaceAvailable;
    GVlockptr     spaceAvailableAccess = &castStream(stream)->shm->spaceAvailableAccess;

    vrb_p         vrbHead              = &castStream(stream)->shm->vrbHead;
    void         *vrbTail              = castStream(stream)->app.vrbTail;

    size_t        offset;
    size_t        dataLength;

    int           done         = 0;

    TRY 
    {
        while (1) 
        {
            offset = (size_t) vrb_data_ptr(vrbHead);

            if (gvAcquire(dataAvailableAccess) == -1)
            {
                THROW(e0, "gvAcquire");
            }

            while ((dataLength = vrb_data_len(vrbHead)) == 0)
            {
                if (gvWait(dataAvailable, dataAvailableAccess) == -1)
                {
                    THROW(e0, "gvWait");
                }
            }

            if (gvRelease(dataAvailableAccess) == -1)
            {
                THROW(e0, "gvRelease");
            }

            if (dataLength >= restLength)
            {
                memcpy(toAddr, vrbTail + offset, restLength);

                if (vrb_take(vrbHead, restLength) == -1)
                {
                    THROW(e0, "vrb_take");
                }

                done = 1;
            }
            else
            {
                memcpy(toAddr, vrbTail + offset, dataLength);
                
                if (vrb_take(vrbHead, dataLength) == -1)
                {
                    THROW(e0, "vrb_take");
                }

                toAddr     = toAddr + dataLength;
                restLength = restLength - dataLength;
            }

            if (gvAcquire(spaceAvailableAccess) == -1)
            {
                THROW(e0, "gvRelease");
            }

            if (gvNotify(spaceAvailable) == -1)
            {
                THROW(e0, "gvNotify");
            }

            if (gvRelease(spaceAvailableAccess) == -1)
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
        return -1;
    }

    return 0;
}

static int
writeFunc(GVstreamptr  stream,
	  const void  *fromAddr,
	  size_t       restLength)
{
    GVcondvarptr  dataAvailable        = &castStream(stream)->shm->dataAvailable;
    GVlockptr     dataAvailableAccess  = &castStream(stream)->shm->dataAvailableAccess;

    GVcondvarptr  spaceAvailable       = &castStream(stream)->shm->spaceAvailable;
    GVlockptr     spaceAvailableAccess = &castStream(stream)->shm->spaceAvailableAccess;

    vrb_p         vrbHead              = &castStream(stream)->shm->vrbHead;
    void         *vrbTail              = castStream(stream)->app.vrbTail;

    size_t        offset;
    size_t        spaceLength;

    int           done         = 0;

    TRY
    {
        while (1) 
        {
            offset = (size_t) vrb_space_ptr(vrbHead);

            if (gvAcquire(spaceAvailableAccess) == -1)
            {
                THROW(e0, "gvAcquire");
            }

            while ((spaceLength = vrb_space_len(vrbHead)) == 0)
            {
                if (gvWait(spaceAvailable, spaceAvailableAccess) == -1)
                {
                    THROW(e0, "gvWait");
                }
            }

            if (gvRelease(spaceAvailableAccess) == -1)
            {
                THROW(e0, "gvRelease");
            }

            if (spaceLength >= restLength)
            {
                memcpy(vrbTail + offset, fromAddr, restLength);
        
                if (vrb_give(vrbHead, restLength) == -1)
                {
                    THROW(e0, "vrb_give");
                }

                done = 1;
            }
            else if (spaceLength > 0)
            {
                memcpy(vrbTail + offset, fromAddr, spaceLength);

                if (vrb_give(vrbHead, spaceLength) == -1)
                {
                    THROW(e0, "vrb_give");
                }

                fromAddr   = fromAddr + spaceLength;
                restLength = restLength - spaceLength;
            }

            if (gvAcquire(dataAvailableAccess) == -1)
            {
                THROW(e0, "gvRelease");
            }

            if (gvNotify(dataAvailable) == -1)
            {
                THROW(e0, "gvNotify");
            }

            if (gvRelease(dataAvailableAccess) == -1)
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
        return -1;
    }

    return 0;
}

static void
*peekFunc(GVstreamptr stream,
	  size_t      numBytes)
{
    GVcondvarptr  dataAvailable        = &castStream(stream)->shm->dataAvailable;
    GVlockptr     dataAvailableAccess  = &castStream(stream)->shm->dataAvailableAccess;

    vrb_p         vrbHead              = &castStream(stream)->shm->vrbHead;
    void         *vrbTail              = castStream(stream)->app.vrbTail;

    size_t        offset;
    size_t        dataLength;

    TRY 
    {
        while (1) 
        {
            offset = (size_t) vrb_data_ptr(vrbHead);

            if (gvAcquire(dataAvailableAccess) == -1)
            {
                THROW(e0, "gvAcquire");
            }

            while ((dataLength = vrb_data_len(vrbHead)) <= numBytes)
            {
                if (gvWait(dataAvailable, dataAvailableAccess) == -1)
                {
                    THROW(e0, "gvWait");
                }
            }

            if (gvRelease(dataAvailableAccess) == -1)
            {
                THROW(e0, "gvRelease");
            }

            break;
        }
    }
    CATCH (e0)
    {
        return NULL;
    }

    return vrbTail + offset;
}

static int
skipFunc(GVstreamptr stream,
	 size_t      length)
{
    TRY
    {
        if (vrb_take(&castStream(stream)->shm->vrbHead, length) == -1)
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

/** ***************************************************************************
 * Constructor / destructor
 */

/* ****************************************************************************
 * File / memory layout 
 */

#define STATE_INITIALIZED   0x1
#define STATE_UNINITIALIZED 0x0
 
/* TODO not portable, use getSystemPageSize() */
static int systemPageSize = 4096;

#define HEAD_SIZE                          systemPageSize
#define TAIL_SIZE(length)                  (length - systemPageSize)
 
#define VRB_HEAD_SIZE                      sizeof(VRB)
#define VRB_TAIL_SIZE(length)              (TAIL_SIZE(length) >> 1)

/* Physical offsets (temp file) */

#define FILE_HEAD_OFFSET                    0
#define FILE_TAIL_OFFSET                    systemPageSize

#define FILE_OS_VRB_TAIL_OFFSET             FILE_TAIL_OFFSET
#define FILE_IS_VRB_TAIL_OFFSET(length)     FILE_OS_VRB_TAIL_OFFSET \
                                            + VRB_TAIL_SIZE(length)
/* Logical offsets (mmap) */

#define MMAP_HEAD_OFFSET                    0
#define MMAP_TAIL_OFFSET                    systemPageSize

#define MMAP_OS_VRB_TAIL_1ST_OFFSET         MMAP_TAIL_OFFSET
#define MMAP_OS_VRB_TAIL_2ND_OFFSET(length) MMAP_TAIL_OFFSET        \
                                            + VRB_TAIL_SIZE(length)

#define MMAP_IS_VRB_TAIL_1ST_OFFSET(length) MMAP_TAIL_OFFSET        \
                                            + TAIL_SIZE(length)
#define MMAP_IS_VRB_TAIL_2ND_OFFSET(length) MMAP_TAIL_OFFSET        \
                                            + TAIL_SIZE(length)     \
                                            + VRB_TAIL_SIZE(length)

static void
*attachShmRegions(GVshmptr shm,
		  size_t   offset,
		  size_t   length)
{
    void *base;

    TRY
    {
	if ((base = mmap(NULL,
			 length + TAIL_SIZE(length),
			 PROT_NONE,
			 MAP_ANONYMOUS | MAP_PRIVATE,
			 -1,
			 0)) == MAP_FAILED)
	{
	    THROW(e0, "mmap");
	}

	if (gvAttachShm(base   + MMAP_HEAD_OFFSET,
			shm,
			offset + FILE_HEAD_OFFSET,
			HEAD_SIZE) == -1)
	{
	    THROW(e0, "gvAttachShm");
	}

	if (gvAttachShm(base   + MMAP_OS_VRB_TAIL_1ST_OFFSET,
			shm,
			offset + FILE_OS_VRB_TAIL_OFFSET,
			TAIL_SIZE(length)) == -1)
	{
	    THROW(e0, "gvAttachShm");
	}

	if (gvAttachShm(base   + MMAP_OS_VRB_TAIL_2ND_OFFSET(length),
			shm,
			offset + FILE_OS_VRB_TAIL_OFFSET,
			TAIL_SIZE(length)) == -1)
	{
	    THROW(e0, "gvAttachShm");
	}

	if (gvAttachShm(base   + MMAP_IS_VRB_TAIL_1ST_OFFSET(length),
			shm,
			offset + FILE_IS_VRB_TAIL_OFFSET(length),
			TAIL_SIZE(length)) == -1)
	{
	    THROW(e0, "gvAttachShm");
	}

	if (gvAttachShm(base   + MMAP_IS_VRB_TAIL_2ND_OFFSET(length),
			shm,
			offset + FILE_IS_VRB_TAIL_OFFSET(length),
			TAIL_SIZE(length)) == -1)
	{
	    THROW(e0, "gvAttachShm");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return base;
}

static int
initStreams(struct Stream *os,
            struct Stream *is,
            size_t         length)
{
    vrb_p osVrbHead = &os->shm->vrbHead;
    vrb_p isVrbHead = &is->shm->vrbHead;

    osVrbHead->lower_ptr = 0;
    osVrbHead->upper_ptr = (void *)VRB_TAIL_SIZE(length);
    osVrbHead->first_ptr = 0;
    osVrbHead->last_ptr  = 0;
    osVrbHead->mem_ptr   = 0;
    osVrbHead->buf_size  = VRB_TAIL_SIZE(length);
    vrb_set_mmap(osVrbHead);

    isVrbHead->lower_ptr = 0;
    isVrbHead->upper_ptr = (void *)VRB_TAIL_SIZE(length);
    isVrbHead->first_ptr = 0;
    isVrbHead->last_ptr  = 0;
    isVrbHead->mem_ptr   = 0;
    isVrbHead->buf_size  = VRB_TAIL_SIZE(length);
    vrb_set_mmap(isVrbHead);

    return 0;
}

static int
initSyncPrimitives(struct Stream *os,
                   struct Stream *is)
{
    TRY
    {
        if (gvCreateLock(&os->shm->exclusiveAccess) == NULL)
        {
            THROW(e0, "gvCreateLock");
        }

        if (gvCreateCondVar(&os->shm->dataAvailable) == NULL)
        {
            THROW(e0, "gvCreateCondVar");
        }

        if (gvCreateLock(&os->shm->dataAvailableAccess) == NULL)
        {
            THROW(e0, "gvCreateLock");
        }

        if (gvCreateCondVar(&os->shm->spaceAvailable) == NULL)
        {
            THROW(e0, "gvCreateCondVar");
        }

        if (gvCreateLock(&os->shm->spaceAvailableAccess) == NULL)
        {
            THROW(e0, "gvCreateLock");
        }

        if (gvCreateLock(&is->shm->exclusiveAccess) == NULL)
        {
            THROW(e0, "gvCreateLock");
        }

        if (gvCreateCondVar(&is->shm->dataAvailable) == NULL)
        {
            THROW(e0, "gvCreateCondVar");
        }

        if (gvCreateLock(&is->shm->dataAvailableAccess) == NULL)
        {
            THROW(e0, "gvCreateLock");
        }

        if (gvCreateCondVar(&is->shm->spaceAvailable) == NULL)
        {
            THROW(e0, "gvCreateCondVar");
        }

        if (gvCreateLock(&is->shm->spaceAvailableAccess) == NULL)
        {
            THROW(e0, "gvCreateLock");
        }
    }
    CATCH (e0)
    {
        return -1;
    }
 
    return 0;
}

GVshmtransportptr
gvCreateShmTransport(GVshmptr shm,
		     size_t   offset,
		     size_t   length)
{
    struct Stream    *is;
    struct Stream    *os;
    struct Transport *trp;

    TRY
    {
	/*
	 * Constraint:
	 *
	 *   transport tail size / 2 = n * system page size, n is positive
	 *
	 */
	if (!(length > 0 && VRB_TAIL_SIZE(length) % systemPageSize == 0))
	{
	    errno = EINVAL;
	    THROW(e0, "invalid length");
	}

	if ((trp = malloc(sizeof(struct Transport))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	if ((trp->shm = attachShmRegions(shm, offset, length)) == NULL)
	{
	    THROW(e0, "attachShmRegions");
	}

	if ((os = malloc(sizeof(struct Stream))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	if ((is = malloc(sizeof(struct Stream))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	os->shm = &trp->shm->os;
	os->app.vrbTail = trp->shm + MMAP_OS_VRB_TAIL_1ST_OFFSET;

	is->shm = &trp->shm->is;
	is->app.vrbTail = trp->shm + MMAP_IS_VRB_TAIL_1ST_OFFSET(length);

	if (trp->shm->state == STATE_UNINITIALIZED)
	{
	    trp->shm->state = STATE_INITIALIZED;

	    if (initStreams(os, is, length) == -1)
            {
                THROW(e0, "initStreams");
            }

            if (initSyncPrimitives(os, is) == -1)
            {
                THROW(e0, "initSyncPrimitives");
            }
	}

 	trp->base.shm    = shm;
	trp->base.offset = offset;
	trp->base.length = length;

	trp->base.base.os = (GVstreamptr) os;
	trp->base.base.is = (GVstreamptr) is;

	trp->base.base.os->exclusiveAccess = &os->shm->exclusiveAccess;
	trp->base.base.is->exclusiveAccess = &is->shm->exclusiveAccess;

	trp->base.base.read  = readFunc;
	trp->base.base.write = writeFunc;
	trp->base.base.peek  = peekFunc;
	trp->base.base.skip  = skipFunc;
    }
    CATCH (e0)
    {
	return NULL;
    }

    return (GVshmtransportptr) trp;
}

int
gvDestroyShmTransport(GVshmtransportptr transport)
{
    size_t length = transport->length;

    TRY
    {
        if (munmap(castTransport(transport)->shm, length + TAIL_SIZE(length)))
        {
            THROW(e0, "munmap");
        }

        free(transport->base.os);
        free(transport->base.is);
        free(transport);
    }
    CATCH (e0)
    {
        return -1;
    }

    return 0;
}
