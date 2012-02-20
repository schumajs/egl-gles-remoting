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

#include <assert.h>
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

	GVlock    condVarAccess;
	GVcondvar notEmpty;
	GVcondvar notFull;
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

	struct ChanelShmPart callChanel;
	struct ChanelShmPart returnChanel;
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
    GVlockptr     condVarAccess = &castChanel(chanel)->shm->condVarAccess;
    GVcondvarptr  notEmpty      = &castChanel(chanel)->shm->notEmpty;
    GVcondvarptr  notFull       = &castChanel(chanel)->shm->notFull;

    vrb_p         vrbHead       = &castChanel(chanel)->shm->vrbHead;
    void         *vrbTail       = castChanel(chanel)->app.vrbTail;

    size_t        offset;
    size_t        dataLength;

    int           done          = 0;

    TRY 
    {
        while (1) 
        {
	    if (gvAcquire(condVarAccess) == -1)
	    {
		THROW(e0, "gvAcquire");
	    }

            while ((dataLength = vrb_data_len(vrbHead)) == 0)
            {
                if (gvWait(notEmpty, condVarAccess) == -1)
                {
                    THROW(e0, "gvWait");
                }
            }

            offset = (size_t) vrb_data_ptr(vrbHead);

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

	    if (gvRelease(condVarAccess) == -1)
	    {
		THROW(e0, "gvRelease");
	    }

            if (gvNotify(notFull) == -1)
            {
                THROW(e0, "gvNotify");
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
    GVlockptr     condVarAccess = &castChanel(chanel)->shm->condVarAccess;
    GVcondvarptr  notEmpty      = &castChanel(chanel)->shm->notEmpty;
    GVcondvarptr  notFull       = &castChanel(chanel)->shm->notFull;

    vrb_p         vrbHead       = &castChanel(chanel)->shm->vrbHead;
    void         *vrbTail       = castChanel(chanel)->app.vrbTail;

    size_t        offset;
    size_t        spaceLength;

    int           done          = 0;

    TRY
    {
        while (1) 
        {
	    if (gvAcquire(condVarAccess) == -1)
	    {
		THROW(e0, "gvAcquire");
	    }

            while ((spaceLength = vrb_space_len(vrbHead)) == 0)
            {
                if (gvWait(notFull, condVarAccess) == -1)
                {
                    THROW(e0, "gvWait");
                }
            }

            offset = (size_t) vrb_space_ptr(vrbHead);

            if (spaceLength >= restLength)
            {
                memcpy(vrbTail + offset, fromAddr, restLength);
        
                if (vrb_give(vrbHead, restLength) == -1)
                {
                    THROW(e0, "vrb_give");
                }

                done = 1;
            }
            else
            {
                memcpy(vrbTail + offset, fromAddr, spaceLength);

                if (vrb_give(vrbHead, spaceLength) == -1)
                {
                    THROW(e0, "vrb_give");
                }

                fromAddr   = fromAddr + spaceLength;
                restLength = restLength - spaceLength;
            }

	    if (gvRelease(condVarAccess) == -1)
	    {
		THROW(e0, "gvRelease");
	    }

            if (gvNotify(notEmpty) == -1)
            {
                THROW(e0, "gvNotify");
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
    GVlockptr     condVarAccess = &castChanel(chanel)->shm->condVarAccess;
    GVcondvarptr  notEmpty      = &castChanel(chanel)->shm->notEmpty;

    vrb_p         vrbHead       = &castChanel(chanel)->shm->vrbHead;
    void         *vrbTail       = castChanel(chanel)->app.vrbTail;

    size_t        offset;
    size_t        dataLength;

    TRY 
    {
        while (1) 
        {
	    if (gvAcquire(condVarAccess) == -1)
	    {
		THROW(e0, "gvAcquire");
	    }
     
	    while ((dataLength = vrb_data_len(vrbHead)) < numBytes)
            {
                if (gvWait(notEmpty, condVarAccess) == -1)
                {
                    THROW(e0, "gvWait");
                }
            }

            offset = (size_t) vrb_data_ptr(vrbHead);

	    if (gvRelease(condVarAccess) == -1)
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

    GVcondvarptr notFull = &castChanel(chanel)->shm->notFull;

    vrb_p        vrbHead = &castChanel(chanel)->shm->vrbHead;

    TRY
    {
        if (vrb_take(vrbHead, length) == -1)
        {
            THROW(e0, "vrb_take");
        }

	if (gvNotify(notFull) == -1)
	{
	    THROW(e0, "gvNotify");
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

/* PhysreturnChanelal offsets (temp file) */

#define FILE_HEAD_OFFSET                    0
#define FILE_TAIL_OFFSET                    systemPageSize

#define FILE_CC_VRB_TAIL_OFFSET             FILE_TAIL_OFFSET
#define FILE_RC_VRB_TAIL_OFFSET(length)     FILE_CC_VRB_TAIL_OFFSET \
                                            + VRB_TAIL_SIZE(length)
/* LogreturnChanelal offsets (mmap) */

#define MMAP_HEAD_OFFSET                    0
#define MMAP_TAIL_OFFSET                    systemPageSize

#define MMAP_CC_VRB_TAIL_1ST_OFFSET         MMAP_TAIL_OFFSET
#define MMAP_CC_VRB_TAIL_2ND_OFFSET(length) MMAP_TAIL_OFFSET        \
                                            + VRB_TAIL_SIZE(length)

#define MMAP_RC_VRB_TAIL_1ST_OFFSET(length) MMAP_TAIL_OFFSET        \
                                            + TAIL_SIZE(length)
#define MMAP_RC_VRB_TAIL_2ND_OFFSET(length) MMAP_TAIL_OFFSET        \
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

	if (gvAttachShm(base   + MMAP_CC_VRB_TAIL_1ST_OFFSET,
			shm,
			offset + FILE_CC_VRB_TAIL_OFFSET,
			VRB_TAIL_SIZE(length)) == -1)
	{
	    THROW(e0, "gvAttachShm");
	}

	if (gvAttachShm(base   + MMAP_CC_VRB_TAIL_2ND_OFFSET(length),
			shm,
			offset + FILE_CC_VRB_TAIL_OFFSET,
			VRB_TAIL_SIZE(length)) == -1)
	{
	    THROW(e0, "gvAttachShm");
	}

	if (gvAttachShm(base   + MMAP_RC_VRB_TAIL_1ST_OFFSET(length),
			shm,
			offset + FILE_RC_VRB_TAIL_OFFSET(length),
			VRB_TAIL_SIZE(length)) == -1)
	{
	    THROW(e0, "gvAttachShm");
	}

	if (gvAttachShm(base   + MMAP_RC_VRB_TAIL_2ND_OFFSET(length),
			shm,
			offset + FILE_RC_VRB_TAIL_OFFSET(length),
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
initChanels(struct Chanel *callChanel,
            struct Chanel *returnChanel,
            size_t         length)
{
    vrb_p ccVrhHead = &callChanel->shm->vrbHead;
    vrb_p rcVrbHead = &returnChanel->shm->vrbHead;

    ccVrhHead->lower_ptr = 0;
    ccVrhHead->upper_ptr = (void *)VRB_TAIL_SIZE(length);
    ccVrhHead->first_ptr = 0;
    ccVrhHead->last_ptr  = 0;
    ccVrhHead->mem_ptr   = 0;
    ccVrhHead->buf_size  = VRB_TAIL_SIZE(length);
    vrb_set_mmap(ccVrhHead);

    rcVrbHead->lower_ptr = 0;
    rcVrbHead->upper_ptr = (void *)VRB_TAIL_SIZE(length);
    rcVrbHead->first_ptr = 0;
    rcVrbHead->last_ptr  = 0;
    rcVrbHead->mem_ptr   = 0;
    rcVrbHead->buf_size  = VRB_TAIL_SIZE(length);
    vrb_set_mmap(rcVrbHead);

    return 0;
}

static int
initSyncPrimitives(struct Chanel *callChanel,
                   struct Chanel *returnChanel)
{
    TRY
    {
        if (gvCreateLock(&callChanel->shm->exclusiveAccess) == NULL)
        {
            THROW(e0, "gvCreateLock");
        }

        if (gvCreateLock(&callChanel->shm->condVarAccess) == NULL)
        {
            THROW(e0, "gvCreateLock");
        }

        if (gvCreateCondVar(&callChanel->shm->notEmpty) == NULL)
        {
            THROW(e0, "gvCreateCondVar");
        }

        if (gvCreateCondVar(&callChanel->shm->notFull) == NULL)
        {
            THROW(e0, "gvCreateCondVar");
        }

        if (gvCreateLock(&returnChanel->shm->exclusiveAccess) == NULL)
        {
            THROW(e0, "gvCreateLock");
        }

        if (gvCreateLock(&returnChanel->shm->condVarAccess) == NULL)
        {
            THROW(e0, "gvCreateLock");
        }

        if (gvCreateCondVar(&returnChanel->shm->notEmpty) == NULL)
        {
            THROW(e0, "gvCreateCondVar");
        }

        if (gvCreateCondVar(&returnChanel->shm->notFull) == NULL)
        {
            THROW(e0, "gvCreateCondVar");
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
    struct Chanel    *returnChanel;
    struct Chanel    *callChanel;
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

	if ((callChanel = malloc(sizeof(struct Chanel))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	if ((returnChanel = malloc(sizeof(struct Chanel))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	callChanel->shm = &trp->shm->callChanel;
	callChanel->app.vrbTail = (void *)trp->shm + MMAP_CC_VRB_TAIL_1ST_OFFSET;

	returnChanel->shm = &trp->shm->returnChanel;
	returnChanel->app.vrbTail = (void *)trp->shm + MMAP_RC_VRB_TAIL_1ST_OFFSET(length);

	if (trp->shm->state == STATE_UNINITIALIZED)
	{
	    trp->shm->state = STATE_INITIALIZED;

	    if (initChanels(callChanel, returnChanel, length) == -1)
            {
                THROW(e0, "initChanels");
            }

            if (initSyncPrimitives(callChanel, returnChanel) == -1)
            {
                THROW(e0, "initSyncPrimitives");
            }
	}

 	trp->base.shm    = shm;
	trp->base.offset = offset;
	trp->base.length = length;

	trp->base.base.callChanel = (GVchanelptr) callChanel;
	trp->base.base.returnChanel = (GVchanelptr) returnChanel;

	trp->base.base.callChanel->exclusiveAccess = &callChanel->shm->exclusiveAccess;
	trp->base.base.returnChanel->exclusiveAccess = &returnChanel->shm->exclusiveAccess;

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

        free(shmStreamTrp->base.callChanel);
        free(shmStreamTrp->base.returnChanel);
        free(shmStreamTrp);
    }
    CATCH (e0)
    {
        return -1;
    }

    return 0;
}
