/*! ***************************************************************************
 * \file    memory_manager.0.c
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
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "sleep.h"
#include "transport.h"

#include "server/dispatcher.h"
#include "server/memory_manager.h"
#include "server/serializer.h"

/* ****************************************************************************
 * Context
 */

struct Context {
    struct GVMMGRcontext  public;
    void                 *vmShmAddr;
    mspace                vmHeap;
    pid_t                 mmgrPid;
    GVTRPtransportptr     mmgrTransport;
    GVDISdispatcherptr    mmgrDispatcher;
};

/* TODO not portable, use getSystemPageSize() */
static int systemPageSize = 4096;

static struct Context *context;

/* ****************************************************************************
 * Dispatching functions
 */

static
void _gvmmgrAlloc()
{
    GVSERcallid callId;
    int         status;
    size_t      length;
    size_t      offset; 

    callId = gvserCall();
    gvserInParameter(&length, sizeof(size_t));
    gvserEndCall();

    status = gvmmgrAlloc(&offset, length);

    gvserReturn(callId);
    gvserReturnValue(&status, sizeof(int));
    gvserOutParameter(&offset, sizeof(size_t));
    gvserEndReturn();
}

static
void _gvmmgrFree()
{
    GVSERcallid callId;
    int         status;
    size_t      offset;

    callId = gvserCall();
    gvserInParameter(&offset, sizeof(size_t));
    gvserEndCall();

    status = gvmmgrFree(offset);

    gvserReturn(callId);
    gvserReturnValue(&status, sizeof(int));
    gvserEndReturn();
}

/* ****************************************************************************
 * Jump table
 */

static GVDISfunc gvmmgrJumpTable[2] = {
    _gvmmgrAlloc,
    _gvmmgrFree
};

/* ****************************************************************************
 * Constructors and destructors
 */

static
void createMmgrResources()
{
    /* Create transport */
    if (gvtrpCreate(&(context->mmgrTransport),
		    context->public.mmgrShm,
		    0,
		    context->public.mmgrShmSize)
	== -1)
    {
	perror("gvtrpCreate");
	exit(2);
    }

    /* Create dispatcher */
    if (gvdisCreate(&(context->mmgrDispatcher),
		    context->mmgrTransport) == -1)
    {
	perror("gvdisCreate");
	exit(2);
    }
}

static
void createVmResources()
{
    /* Find an appropriate logical memory region */
    if ((context->vmShmAddr
	 = mmap(NULL,
		context->public.vmShmSize,
		PROT_NONE,
		MAP_ANONYMOUS | MAP_PRIVATE,
		-1,
		0)
	    ) == MAP_FAILED)
    {
	perror("mmap");
	exit(2);
    }

    /* Attach shared memory to that region */
    if (gvshmAttach(context->vmShmAddr,
		    context->public.vmShm,
		    0,
		    context->public.vmShmSize)
	== -1)
    {
	perror("gvshmAttach");
	exit(2);
    }

    /* Create dlmalloc mspace */
    if ((context->vmHeap
	 = create_mspace_with_base(context->vmShmAddr,
				   context->public.vmShmSize,
				   0))
	== NULL)
    {
	perror("create_mspace_with_base");
	exit(2);
    }
}

static
void destroyMmgrResources()
{
    /* Destroy dispatcher */
    if ((gvdisDestroy(context->mmgrDispatcher) == -1))
    {
	perror("gvdisDestroy");
	exit(2);
    }

    /* Destroy transport (+ detach shared memory) */
    if (gvtrpDestroy(context->mmgrTransport) == -1)
    {
	perror("gvtrpDestroy");
	exit(2);
    }

    /* Destroy shared memory */
    if (gvshmDestroy(context->public.mmgrShm) == -1)
    {
	perror("gvshmDestroy");
	exit(2);
    }
}

static
void destroyVmResources()
{
    /* Destroy dlmalloc mspace */
    if (destroy_mspace(context->vmHeap) == -1)
    {
	perror("destroy_mspace");
	exit(2);
    }    

    /* Detach shared memory */
    if (gvshmDetach(context->vmShmAddr,
		    context->public.vmShm,
		    0,
		    context->public.vmShmSize)
	== -1)
    {
	perror("gvshmDetach");
	exit(2);
    }

    /* Destroy shared memory*/
    if (gvshmDestroy(context->public.vmShm) == -1)
    {
	perror("gvshmDestroy");
	exit(2);
    }
}

/* ****************************************************************************
 * Memory manager server implementation
 */

static void
sigtermHandler()
{
    destroyMmgrResources();
    destroyVmResources();

    free(context);

    exit(3);
}

int
gvmmgrCreate(GVMMGRcontextptr *newMmgr, size_t heapSize)
{
    if ((context = malloc(sizeof(struct Context))) == NULL)
    {
	perror("malloc");
	return -1;
    }

    /*
     * Create shared memory in parent process, so corresponding file
     * descriptor will be available to child processes.
     */

    context->public.vmShmSize = heapSize;

    if (gvshmCreate(&(context->public.vmShm),
		    context->public.vmShmSize) == -1)
    {
	perror("gvshmCreate");
	return -1;
    }

    /* Min. transport size ... */
    context->public.mmgrShmSize = 3 * 4096;

    if (gvshmCreate(&(context->public.mmgrShm),
		    context->public.mmgrShmSize) == -1)
    {
	perror("gvshmCreate");
	return -1;
    }

    if (!(context->mmgrPid = fork()))
    {
	/*
         * Memory manager process
         */

	/* Register "shutdown hook" */
	if (signal(SIGTERM, sigtermHandler) == SIG_ERR)
	{
	    perror("signal");
	    exit(2);
	}

	/* Attach VM shared memory, create dlmalloc heap */
	createVmResources();

	/* Create memory manager transport, dispatcher */
	createMmgrResources();
		
	/* Start dispatching loop */
	if (gvdisMakeCurrent(context->mmgrDispatcher) == -1)
	{
	    perror("gvdisMakeCurrent");
	    exit(2);
	}

	if (gvdisDispatchLoop(NULL, gvmmgrJumpTable, 1) == -1)
	{
	    perror("gvdisDispatchLoop");
	    exit(2);
	}

	return 0;
    }
    else if (context->mmgrPid > 0)
    {
	/*
	 * Dashboard process
         */

	*newMmgr = (GVMMGRcontextptr) context;

	return 1;
    }
    else if (context->mmgrPid < 0)
    {
	perror("fork");
    }

    return -1;
}

int
gvmmgrAlloc(size_t *offset, size_t length)
{
    void *addr;

    if ((addr = mspace_memalign(context->vmHeap, systemPageSize, length))
	== NULL)
    {
	perror("mspace_malloc");
	return -1;
    }

    *offset = addr - context->vmShmAddr;

    return 0;
}

int
gvmmgrFree(size_t offset)
{
    mspace_free(context->vmHeap, context->vmShmAddr + offset);

    return 0;
}

int
gvmmgrDestroy(GVMMGRcontextptr mmgr)
{
    if (kill(((struct Context *)context)->mmgrPid, SIGTERM) == -1)
    {
	perror("kill");
	return -1;
    }

    free(context);

    return 0;
}
