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
    char   mmgrShmFd[12];
    char   mmgrShmSize[12];
    char   janitorShmFd[12];
    char   janitorShmSize[12];

    snprintf(vmShmFd, 12, "%i", heapMgr->vmShm->id);
    snprintf(vmShmSize, 12, "%zu", heapMgr->vmShm->size);

    snprintf(janitorShmFd, 12, "%i", janitor->janitorShm->id);
    snprintf(janitorShmSize, 12, "%zu", janitor->janitorShm->size);

    if ((demoClientPid = fork()) == 0)
    {
	char *env[7] = {
	    vmShmFd,
	    vmShmSize,
	    mmgrShmFd,
	    mmgrShmSize,
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

	gvSleep(10, 0);

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
	puts("ERROR");
    }

    puts("SUCCESS");
}

/*
static void
democlient()
{
    pid_t  pid;

    char   vmShmFd[12];
    char   vmShmSize[12];
    char   mmgrShmFd[12];
    char   mmgrShmSize[12];
    char   srvShmFd[12];
    char   srvShmSize[12];

    snprintf(vmShmFd, 12, "%i", *(mmgr->vmShm));
    snprintf(vmShmSize, 12, "%zu", mmgr->vmShmSize);

    snprintf(mmgrShmFd, 12, "%i", *(mmgr->mmgrShm));
    snprintf(mmgrShmSize, 12, "%zu", mmgr->mmgrShmSize);

    snprintf(srvShmFd, 12, "%i", *(server->srvShm));
    snprintf(srvShmSize, 12, "%zu", server->srvShmSize);

    if ((pid = fork()) == 0)
    {
	char *env[7] = {
	    vmShmFd,
	    vmShmSize,
	    mmgrShmFd,
	    mmgrShmSize,
	    srvShmFd,
	    srvShmSize,
	    0
	};

	if (execvpe("./gvdc", NULL, env) == -1)
	{
	    perror("execvpe");
	}
    }
}
*/

/*
int main()
{
    int status;

    if ((status = gvmmgrCreate(&mmgr, 1UL << 25)) == -1)
    {
	perror("gvmmgrCreate");
	exit(2);
    }

    if ((status = gvsrvCreate(&server, mmgr->vmShm)) == -1)
    {
	perror("gvsrvCreate");
	exit(2);
    }

    democlient();

    gvslpSleep(5, 0);

    if ((status = gvsrvDestroy(server)) == -1)
    {
	perror("gvsrvDestroy");
	exit(2);
    }

    if ((status = gvmmgrDestroy(mmgr)) == -1)
    {
	perror("gvmmgrDestroy");
	exit(2);
    }

    return 0;
}
*/
