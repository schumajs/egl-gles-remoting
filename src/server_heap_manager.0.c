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
#include "serializer.h"
#include "server_dispatcher.h"
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
    int            status;
    size_t         length;
    size_t         offset; 

    TRY
    {
	if (gvCall(transport, NULL, NULL, &callId) == -1)
	{
	    THROW(e0, "gvCall");
	}

	if (gvGetData(transport, &length, sizeof(size_t)) == -1)
	{
	    THROW(e0, "gvGetData");
	}

	if (gvEndCall(transport, NULL) == -1) {
	    THROW(e0, "gvEndCall");
	}

	status = gvAlloc(&offset, length);

	if (gvReturn(transport, NULL, &callId) == -1)
	{
	    THROW(e0, "gvReturn");
	}

	if (gvPutData(transport, &status, sizeof(int)) == -1)
	{
	    THROW(e0, "gvPutData");
	}

	if (gvPutData(transport, &offset, sizeof(size_t)) == -1)
	{
	    THROW(e0, "gvPutData");
	}

	if (gvEndReturn(transport, NULL) == -1)
	{
	    THROW(e0, "gvReturn");
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
	if (gvCall(transport, NULL, NULL, &callId) == -1)
	{
	    THROW(e0, "gvCall");
	}

	if (gvGetData(transport, &offset, sizeof(size_t)) == -1)
	{
	    THROW(e0, "gvGetData");
	}

	if (gvEndCall(transport, NULL) == -1) {
	    THROW(e0, "gvEndCall");
	}

	if (gvReturn(transport, NULL, &callId) == -1)
	{
	    THROW(e0, "gvReturn");
	}

	if (gvPutData(transport, &status, sizeof(int)) == -1)
	{
	    THROW(e0, "gvPutData");
	}

	if (gvEndReturn(transport, NULL) == -1)
	{
	    THROW(e0, "gvReturn");
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
	if (gvCreateTransport(&(heapMgr->heapMgrTransport),
			      heapMgr->public.heapMgrShm,
			      0,
			      heapMgr->public.heapMgrShm->size) == -1)
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

int
gvStartHeapMgr(GVheapmgrptr *newHeapMgr, size_t heapSize)
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

	if (gvCreateShm(&(heapMgr->public.vmShm), heapSize) == -1)
	{
	    THROW(e0, "gvCreateShm");
	}

	if (gvCreateShm(&(heapMgr->public.heapMgrShm), 3 * 4096) == -1)
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

	    return 0;
	}
	else if (heapMgr->heapMgrPid > 0)
	{
	    /*
	     * Dashboard process
	     */

	    *newHeapMgr = (GVheapmgrptr) heapMgr;

	    return 1;
	}
	else if (heapMgr->heapMgrPid < 0)
	{
	    THROW(e0, "fork");
	}
    }
    CATCH (e0)
    {
	return -1;
    }
    CATCH (e1)
    {
	exit(0);
    }

    return -1;
}

int
gvAlloc(size_t *offset, size_t length)
{
    void *addr;

    TRY
    {
	if ((addr
	     = mspace_memalign(heapMgr->vmHeap, systemPageSize, length)) == NULL)
	{
	    THROW(e0, "mspace_memalign");
	}

	*offset = addr - heapMgr->vmShmAddr;
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
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