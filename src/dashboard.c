#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "shared_memory.h"
#include "sleep.h"
#include "server/memory_manager.h"
#include "server/server.h"

static GVMMGRcontextptr mmgr;
static GVSRVcontextptr  server;

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

    snprintf(vmShmFd, 12, "%i", *((int *)mmgr->vmShm));
    snprintf(vmShmSize, 12, "%zu", mmgr->vmShmSize);

    snprintf(mmgrShmFd, 12, "%i", *((int *)mmgr->mmgrShm));
    snprintf(mmgrShmSize, 12, "%zu", mmgr->mmgrShmSize);

    snprintf(srvShmFd, 12, "%i", *((int *)server->srvShm));
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

    gvslpSleep(1, 0);

    democlient();

    gvslpSleep(10, 0);

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
