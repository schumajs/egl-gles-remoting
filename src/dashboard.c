#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "error.h"
#include "shared_memory.h"
#include "server_heap_manager.h"
#include "server_janitor.h"
#include "sleep.h"

extern char **environ;

static GVheapmgrptr heapMgr;
static GVjanitorptr janitor;

static void
demoClient()
{
    if (fork() == 0)
    {
	char   vmShmFd[12];
	char   vmShmSize[12];
	char   hmgrShmFd[12];
	char   hmgrShmSize[12];
	char   janitorShmFd[12];
	char   janitorShmSize[12];

	snprintf(vmShmFd, 12, "%i", heapMgr->vmShm->id);
	snprintf(vmShmSize, 12, "%zu", heapMgr->vmShm->size);
	setenv("GV_VM_SHM_FD", vmShmFd, 0);
	setenv("GV_VM_SHM_SIZE", vmShmSize, 0);

	snprintf(hmgrShmFd, 12, "%i", heapMgr->heapMgrShm->id);
	snprintf(hmgrShmSize, 12, "%zu", heapMgr->heapMgrShm->size);
	setenv("GV_HMGR_SHM_FD", hmgrShmFd, 0);
	setenv("GV_HMGR_SHM_SIZE", hmgrShmSize, 0);

	snprintf(janitorShmFd, 12, "%i", janitor->janitorShm->id);
	snprintf(janitorShmSize, 12, "%zu", janitor->janitorShm->size);
	setenv("GV_JANITOR_SHM_FD", janitorShmFd, 0);
	setenv("GV_JANITOR_SHM_SIZE", janitorShmSize, 0);
      
	TRY
	{
	    if (execvpe("./eval1", NULL, environ) == -1)
	    {
		THROW(e0, "execvpe");
	    }
	}
	CATCH (e0)
	{
	    return;
	}
    }
}

int main()
{
    TRY
    {
	if ((heapMgr = gvStartHeapMgr(1UL << 20)) == NULL)
	{
	    THROW(e0, "gvStartHeapMgr");
	}

	if ((janitor = gvStartJanitor(heapMgr->vmShm)) == NULL)
	{
	    THROW(e0, "gvStartJanitor");
	}

	gvSleep(1, 0);

	demoClient();

	gvSleep(180, 0);

	if (gvStopJanitor(janitor) == -1)
	{
	    THROW(e0, "gvStopJanitor");
	}

	if (gvStopHeapMgr(heapMgr) == -1)
	{
	    THROW(e0, "gvStopHeapMgr");
	}
    }
    CATCH (e0)
    {
	puts("DASHBOARD ERROR");
	return -1;
    }

    puts("DASHBOARD SUCCESS");
    return 0;
}
