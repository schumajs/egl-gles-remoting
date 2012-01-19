#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "error.h"
#include "shared_memory.h"
#include "server_heap_manager.h"
#include "server_janitor.h"
#include "sleep.h"

#include "server_dispatcher.h"
GVdispatchfunc eglGlesJumpTable[0] = {

};

static GVheapmgrptr heapMgr;
static GVjanitorptr janitor;

static void
demoClient()
{
    pid_t  demoClientPid;

    char   vmShmFd[12];
    char   vmShmSize[12];
    char   hmgrShmFd[12];
    char   hmgrShmSize[12];
    char   janitorShmFd[12];
    char   janitorShmSize[12];

    snprintf(vmShmFd, 12, "%i", heapMgr->vmShm->id);
    snprintf(vmShmSize, 12, "%zu", heapMgr->vmShm->size);

    snprintf(hmgrShmFd, 12, "%i", heapMgr->heapMgrShm->id);
    snprintf(hmgrShmSize, 12, "%zu", heapMgr->heapMgrShm->size);

    snprintf(janitorShmFd, 12, "%i", janitor->janitorShm->id);
    snprintf(janitorShmSize, 12, "%zu", janitor->janitorShm->size);

    if ((demoClientPid = fork()) == 0)
    {
	char *env[7] = {
	    vmShmFd,
	    vmShmSize,
	    hmgrShmFd,
	    hmgrShmSize,
	    janitorShmFd,
	    janitorShmSize,
	    0
	};

	if (execvpe("./gvdc", NULL, env) == -1)
	{
	    perror("execvpe");
	}
    }
}

int main()
{
    TRY
    {
	if (gvStartHeapMgr(&heapMgr, 1UL << 20) == -1)
	{
	    THROW(e0, "gvStartHeapMgr");
	}

	if (gvStartJanitor(&janitor, heapMgr->vmShm) == -1)
	{
	    THROW(e0, "gvStartJanitor");
	}

	gvSleep(2, 0);

	demoClient();

	gvSleep(20, 0);

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
