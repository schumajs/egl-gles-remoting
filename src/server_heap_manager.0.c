/*! ***************************************************************************
 * \file    server_heap_manager.0.c
 * \brief   
 * 
 * \date    December 25, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#define ONLY_MSPACES 1
#define MSPACES 1
#include <dlmalloc.h>

#include <signal.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "error.h"
#include "server_dispatcher.h"
#include "server_serializer.h"
#include "server_heap_manager.h"
#include "transport.h"

/* ****************************************************************************
 * Context
 */

struct HeapMgr {
    struct GVheapmgr  public;

    void             *vmShmAddr;
    mspace            vmHeap;

    pid_t             heapMgrPid;
    GVtransportptr    heapMgrTransport;
};

#define castHeapMgr(heapMgr) \
    ((struct HeapMgr *)heapMgr)

/* TODO not portable, use getSystemPageSize() */
static int systemPageSize = 4096;

static struct HeapMgr *heapMgr;

/* ****************************************************************************
 * Dispatching functions
 */

static
void _gvAlloc()
{
    GVtransportptr transport = heapMgr->heapMgrTransport;

    GVcallid       callId;
    size_t         offset; 
    size_t         length;

    TRY
    {
	if (gvReceiveData(transport, &callId, sizeof(GVcallid)) == -1)
	{
	    THROW(e0, "gvReceiveDataData");
	}

	if (gvReceiveData(transport, &length, sizeof(size_t)) == -1)
	{
	    THROW(e0, "gvReceiveData");
	}

	offset = gvAlloc(length);

	if (gvStartSending(transport, NULL, callId) == -1)
	{
	    THROW(e0, "gvReturn");
	}

	if (gvSendData(transport, &offset, sizeof(size_t)) == -1)
	{
	    THROW(e0, "gvPutData");
	}
    }
    CATCH (e0)
    {
	return;
    }
}

static
void _gvFree()
{
    GVtransportptr transport = heapMgr->heapMgrTransport;

    GVcallid       callId;
    int            status;
    size_t         offset;

    TRY
    {
	if (gvReceiveData(transport, &callId, sizeof(GVcallid)) == -1)
	{
	    THROW(e0, "gvReceiveDataData");
	}

	if (gvReceiveData(transport, &offset, sizeof(size_t)) == -1)
	{
	    THROW(e0, "gvGetData");
	}

	status = gvFree(offset);

	if (gvStartSending(transport, NULL, callId) == -1)
	{
	    THROW(e0, "gvReturn");
	}

	if (gvSendData(transport, &status, sizeof(int)) == -1)
	{
	    THROW(e0, "gvPutData");
	}
    }
    CATCH (e0)
    {
	return;
    }
}

/* ****************************************************************************
 * Jump table
 */

static GVdispatchfunc gvHeapMgrJumpTable[2] = {
    _gvAlloc,
    _gvFree
};

/* ****************************************************************************
 * Constructors and destructors
 */

static
void createHeapMgrResources()
{
    TRY
    {
	/* Create transport */
	if ((heapMgr->heapMgrTransport
	     = gvCreateTransport(heapMgr->public.heapMgrShm,
				 0,
				 heapMgr->public.heapMgrShm->size)) == NULL)
	{
	    THROW(e0, "gvCreateTransport");
	}
    }
    CATCH (e0)
    {
	exit(2);
    }
}

static
void createVmResources()
{
    TRY
    {
	/* Find an appropriate logical memory region */
	if ((heapMgr->vmShmAddr
	     = mmap(NULL,
		    heapMgr->public.vmShm->size,
		    PROT_NONE,
		    MAP_ANONYMOUS | MAP_PRIVATE,
		    -1,
		    0)
		) == MAP_FAILED)
	{
	    THROW(e0, "mmap");
	}

	/* Attach shared memory to that region */
	if (gvAttachShm(heapMgr->vmShmAddr,
			heapMgr->public.vmShm,
			0,
			heapMgr->public.vmShm->size) == -1)
	{
	    THROW(e0, "gvAttachShm");
	}

	/* Create dlmalloc mspace */
	if ((heapMgr->vmHeap
	     = create_mspace_with_base(heapMgr->vmShmAddr,
				       heapMgr->public.vmShm->size,
				       0)) == NULL)
	{
	    THROW(e0, "create_mspace_with_base");
	}	
    }
    CATCH (e0)
    {
	exit(2);
    }
}

static
void destroyHeapMgrResources()
{
    TRY
    {
	/* Destroy transport (+ detach shared memory) */
	if (gvDestroyTransport(heapMgr->heapMgrTransport) == -1)
	{
	    THROW(e0, "gvDestroyTransport");
	}

	/* Destroy shared memory */
	if (gvDestroyShm(heapMgr->public.heapMgrShm) == -1)
	{
	    THROW(e0, "gvDestroyShm");
	}
    }
    CATCH (e0)
    {
	exit(2);
    }
}

static
void destroyVmResources()
{
    TRY
    {
	/* Destroy dlmalloc mspace */
	if (destroy_mspace(heapMgr->vmHeap) == -1)
	{
	    THROW(e0, "destroy_mspace");
	}    

	/* Detach shared memory */
	if (gvDetachShm(heapMgr->vmShmAddr,
			heapMgr->public.vmShm,
			0,
			heapMgr->public.vmShm->size) == -1)
	{
	    THROW(e0, "gvDetachShm");
	}

	/* Destroy shared memory*/
	if (gvDestroyShm(heapMgr->public.vmShm) == -1)
	{
	    THROW(e0, "gvDestroyShm");
	}	
    }
    CATCH (e0)
    {
	exit(2);
    }
}

/* ****************************************************************************
 * Memory manager server implementation
 */

static void
sigtermHandler()
{
    destroyHeapMgrResources();
    destroyVmResources();
    free(heapMgr);
    exit(3);
}

GVheapmgrptr
gvStartHeapMgr(size_t heapSize)
{
    TRY
    {
	if ((heapMgr = malloc(sizeof(struct HeapMgr))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	/* Create shared memory in parent process, so corresponding file
	 * descriptor will be available to child processes.
	 */

	if ((heapMgr->public.vmShm = gvCreateShm(heapSize)) == NULL)
	{
	    THROW(e0, "gvCreateShm");
	}

	if ((heapMgr->public.heapMgrShm = gvCreateShm(3 * 4096)) == NULL)
	{
	    THROW(e0, "gvCreateShm");
	}	

	if (!(heapMgr->heapMgrPid = fork()))
	{
	    /*
	     * Memory manager process
	     */

	    /* Register "shutdown hook" */
	    if (signal(SIGTERM, sigtermHandler) == SIG_ERR)
	    {
		THROW(e1, "signal");
	    }

	    /* Attach VM shared memory, create dlmalloc heap */
	    createVmResources();

	    /* Create memory manager transport, dispatcher */
	    createHeapMgrResources();

	    /* Start dispatching loop */	
	    if (gvDispatchLoop(heapMgr->heapMgrTransport,
			       gvHeapMgrJumpTable,
			       1) == -1)
	    {
		THROW(e1, "gvDispatchLoop");
	    }

	    return NULL;
	}
	else if (heapMgr->heapMgrPid > 0)
	{
	    return (GVheapmgrptr) heapMgr;
	}
	else if (heapMgr->heapMgrPid < 0)
	{
	    THROW(e0, "fork");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }
    CATCH (e1)
    {
	exit(0);
    }

    return NULL;
}

size_t
gvAlloc(size_t length)
{
    void   *addr;
    size_t  offset;

    TRY
    {
	if ((addr
	     = mspace_memalign(heapMgr->vmHeap, systemPageSize, length)) == NULL)
	{
	    THROW(e0, "mspace_memalign");
	}

	offset = addr - heapMgr->vmShmAddr;
    }
    CATCH (e0)
    {
	return 0;
    }

    return offset;
}

int
gvFree(size_t offset)
{    
    mspace_free(heapMgr->vmHeap, heapMgr->vmShmAddr + offset);

    return 0;
}

int
gvStopHeapMgr(GVheapmgrptr heapMgr)
{
    TRY
    {
	if (kill(castHeapMgr(heapMgr)->heapMgrPid, SIGTERM) == -1)
	{
	    THROW(e0, "kill");
	}

	free(heapMgr);
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}
