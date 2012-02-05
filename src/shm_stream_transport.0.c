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
#include "shm_stream_transport.h"

/* ****************************************************************************
 * GVchanel implementation
 */

struct Chanel {

    /* App. address space */
    struct GVchanel  base;

    struct AppPart {
	void        *vrbTail;
    } app;
    
    /* Shared memory */
    struct ChanelShmPart {
	VRB       vrbHead;

	GVlock    exclusiveAccess;

	GVcondvar dataAvailable;
	GVlock    dataAvailableAccess;

	GVcondvar spaceAvailable;
	GVlock    spaceAvailableAccess;
    } *shm;

};

#define castChanel(chanel) \
    ((struct Chanel *)chanel)

/* ****************************************************************************
 * GVtransport implementation
 */

struct Transport {

    /* App. address space */
    struct GVshmstreamtrp base;

    /* Shared memory */
    struct TransportShmPart {
	int                  state;

	struct ChanelShmPart oc;
	struct ChanelShmPart ic;
    } *shm;

};

#define castTransport(transport) \
    ((struct Transport *)transport)

/** ***************************************************************************
 * Operations
 */

static int
readFunc(GVchanelptr  chanel,
	 void        *toAddr,
	 size_t       restLength)
{
    GVcondvarptr  dataAvailable        = &castChanel(chanel)->shm->dataAvailable;
    GVlockptr     dataAvailableAccess  = &castChanel(chanel)->shm->dataAvailableAccess;

    GVcondvarptr  spaceAvailable       = &castChanel(chanel)->shm->spaceAvailable;
    GVlockptr     spaceAvailableAccess = &castChanel(chanel)->shm->spaceAvailableAccess;

    vrb_p         vrbHead              = &castChanel(chanel)->shm->vrbHead;
    void         *vrbTail              = castChanel(chanel)->app.vrbTail;

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
writeFunc(GVchanelptr  chanel,
	  const void  *fromAddr,
	  size_t       restLength)
{
    GVcondvarptr  dataAvailable        = &castChanel(chanel)->shm->dataAvailable;
    GVlockptr     dataAvailableAccess  = &castChanel(chanel)->shm->dataAvailableAccess;

    GVcondvarptr  spaceAvailable       = &castChanel(chanel)->shm->spaceAvailable;
    GVlockptr     spaceAvailableAccess = &castChanel(chanel)->shm->spaceAvailableAccess;

    vrb_p         vrbHead              = &castChanel(chanel)->shm->vrbHead;
    void         *vrbTail              = castChanel(chanel)->app.vrbTail;

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
*peekFunc(GVchanelptr chanel,
	  size_t      numBytes)
{
    GVcondvarptr  dataAvailable        = &castChanel(chanel)->shm->dataAvailable;
    GVlockptr     dataAvailableAccess  = &castChanel(chanel)->shm->dataAvailableAccess;

    vrb_p         vrbHead              = &castChanel(chanel)->shm->vrbHead;
    void         *vrbTail              = castChanel(chanel)->app.vrbTail;

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
takeFunc(GVchanelptr chanel,
	 size_t      length)
{
    TRY
    {
        if (vrb_take(&castChanel(chanel)->shm->vrbHead, length) == -1)
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

#define FILE_OC_VRB_TAIL_OFFSET             FILE_TAIL_OFFSET
#define FILE_IC_VRB_TAIL_OFFSET(length)     FILE_OC_VRB_TAIL_OFFSET \
                                            + VRB_TAIL_SIZE(length)
/* Logical offsets (mmap) */

#define MMAP_HEAD_OFFSET                    0
#define MMAP_TAIL_OFFSET                    systemPageSize

#define MMAP_OC_VRB_TAIL_1ST_OFFSET         MMAP_TAIL_OFFSET
#define MMAP_OC_VRB_TAIL_2ND_OFFSET(length) MMAP_TAIL_OFFSET        \
                                            + VRB_TAIL_SIZE(length)

#define MMAP_IC_VRB_TAIL_1ST_OFFSET(length) MMAP_TAIL_OFFSET        \
                                            + TAIL_SIZE(length)
#define MMAP_IC_VRB_TAIL_2ND_OFFSET(length) MMAP_TAIL_OFFSET        \
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

	if (gvAttachShm(base   + MMAP_OC_VRB_TAIL_1ST_OFFSET,
			shm,
			offset + FILE_OC_VRB_TAIL_OFFSET,
			VRB_TAIL_SIZE(length)) == -1)
	{
	    THROW(e0, "gvAttachShm");
	}

	if (gvAttachShm(base   + MMAP_OC_VRB_TAIL_2ND_OFFSET(length),
			shm,
			offset + FILE_OC_VRB_TAIL_OFFSET,
			VRB_TAIL_SIZE(length)) == -1)
	{
	    THROW(e0, "gvAttachShm");
	}

	if (gvAttachShm(base   + MMAP_IC_VRB_TAIL_1ST_OFFSET(length),
			shm,
			offset + FILE_IC_VRB_TAIL_OFFSET(length),
			VRB_TAIL_SIZE(length)) == -1)
	{
	    THROW(e0, "gvAttachShm");
	}

	if (gvAttachShm(base   + MMAP_IC_VRB_TAIL_2ND_OFFSET(length),
			shm,
			offset + FILE_IC_VRB_TAIL_OFFSET(length),
			VRB_TAIL_SIZE(length)) == -1)
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
initChanels(struct Chanel *oc,
            struct Chanel *ic,
            size_t         length)
{
    vrb_p ocVrbHead = &oc->shm->vrbHead;
    vrb_p icVrbHead = &ic->shm->vrbHead;

    ocVrbHead->lower_ptr = 0;
    ocVrbHead->upper_ptr = (void *)VRB_TAIL_SIZE(length);
    ocVrbHead->first_ptr = 0;
    ocVrbHead->last_ptr  = 0;
    ocVrbHead->mem_ptr   = 0;
    ocVrbHead->buf_size  = VRB_TAIL_SIZE(length);
    vrb_set_mmap(ocVrbHead);

    icVrbHead->lower_ptr = 0;
    icVrbHead->upper_ptr = (void *)VRB_TAIL_SIZE(length);
    icVrbHead->first_ptr = 0;
    icVrbHead->last_ptr  = 0;
    icVrbHead->mem_ptr   = 0;
    icVrbHead->buf_size  = VRB_TAIL_SIZE(length);
    vrb_set_mmap(icVrbHead);

    return 0;
}

static int
initSyncPrimitives(struct Chanel *oc,
                   struct Chanel *ic)
{
    TRY
    {
        if (gvCreateLock(&oc->shm->exclusiveAccess) == NULL)
        {
            THROW(e0, "gvCreateLock");
        }

        if (gvCreateCondVar(&oc->shm->dataAvailable) == NULL)
        {
            THROW(e0, "gvCreateCondVar");
        }

        if (gvCreateLock(&oc->shm->dataAvailableAccess) == NULL)
        {
            THROW(e0, "gvCreateLock");
        }

        if (gvCreateCondVar(&oc->shm->spaceAvailable) == NULL)
        {
            THROW(e0, "gvCreateCondVar");
        }

        if (gvCreateLock(&oc->shm->spaceAvailableAccess) == NULL)
        {
            THROW(e0, "gvCreateLock");
        }

        if (gvCreateLock(&ic->shm->exclusiveAccess) == NULL)
        {
            THROW(e0, "gvCreateLock");
        }

        if (gvCreateCondVar(&ic->shm->dataAvailable) == NULL)
        {
            THROW(e0, "gvCreateCondVar");
        }

        if (gvCreateLock(&ic->shm->dataAvailableAccess) == NULL)
        {
            THROW(e0, "gvCreateLock");
        }

        if (gvCreateCondVar(&ic->shm->spaceAvailable) == NULL)
        {
            THROW(e0, "gvCreateCondVar");
        }

        if (gvCreateLock(&ic->shm->spaceAvailableAccess) == NULL)
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

GVtransportptr
gvCreateShmStreamTransport(GVshmptr shm,
			   size_t   offset,
			   size_t   length)
{
    struct Chanel    *ic;
    struct Chanel    *oc;
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

	if ((oc = malloc(sizeof(struct Chanel))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	if ((ic = malloc(sizeof(struct Chanel))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	oc->shm = &trp->shm->oc;
	oc->app.vrbTail = (void *)trp->shm + MMAP_OC_VRB_TAIL_1ST_OFFSET;

	ic->shm = &trp->shm->ic;
	ic->app.vrbTail = (void *)trp->shm + MMAP_IC_VRB_TAIL_1ST_OFFSET(length);

	if (trp->shm->state == STATE_UNINITIALIZED)
	{
	    trp->shm->state = STATE_INITIALIZED;

	    if (initChanels(oc, ic, length) == -1)
            {
                THROW(e0, "initChanels");
            }

            if (initSyncPrimitives(oc, ic) == -1)
            {
                THROW(e0, "initSyncPrimitives");
            }
	}

 	trp->base.shm    = shm;
	trp->base.offset = offset;
	trp->base.length = length;

	trp->base.base.oc = (GVchanelptr) oc;
	trp->base.base.ic = (GVchanelptr) ic;

	trp->base.base.oc->exclusiveAccess = &oc->shm->exclusiveAccess;
	trp->base.base.ic->exclusiveAccess = &ic->shm->exclusiveAccess;

	trp->base.base.read  = readFunc;
	trp->base.base.take  = takeFunc;
	trp->base.base.write = writeFunc;
	trp->base.base.peek  = peekFunc;
    }
    CATCH (e0)
    {
	return NULL;
    }

    return (GVtransportptr) trp;
}

int
gvDestroyShmStreamTransport(GVtransportptr transport)
{
    GVshmstreamtrpptr shmStreamTrp = (GVshmstreamtrpptr) transport;

    size_t length = shmStreamTrp->length;

    TRY
    {
        if (munmap(castTransport(shmStreamTrp)->shm, length + TAIL_SIZE(length)))
        {
            THROW(e0, "munmap");
        }

        free(shmStreamTrp->base.oc);
        free(shmStreamTrp->base.ic);
        free(shmStreamTrp);
    }
    CATCH (e0)
    {
        return -1;
    }

    return 0;
}
