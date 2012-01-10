/*! ***************************************************************************
 * \file    shared_memory.0.c
 * \brief   
 * 
 * \date    December 20, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "shared_memory.h"

int
gvshmCreate(GVSHMshmptr *newShm, size_t shmSize)
{
    int  fd;
    char fileName[] = "/tmp/gvshm.XXXXXX";
	
    if ((fd = mkstemp(fileName)) == -1)
    {
	perror("mkstemp");
	return -1;
    }

    if (unlink(fileName) == -1)
    {
	perror("unlink");
	return -1;
    }

    if (ftruncate(fd, shmSize) == -1)
    {
	perror("ftruncate");
	return -1;
    }

    *newShm = malloc(sizeof(GVSHMshm));
    *(*newShm) = fd;

    return 0;
}

int
gvshmAttach(void *toAddr, GVSHMshmptr shm, off_t offset, size_t length)
{
    if (mmap(toAddr, length, PROT_READ | PROT_WRITE,
	     MAP_FIXED | MAP_SHARED, *shm, offset) == MAP_FAILED)
    {
	perror("mmap");
	return -1;
    }

    return 0;
}

int
gvshmDetach(void *fromAddr, GVSHMshmptr shm, off_t offset, size_t length)
{
    if (munmap(fromAddr, length))
    {
	perror("munmap");
	return -1;
    }

    return 0;
}

int
gvshmDestroy(GVSHMshmptr shm)
{
    if (close(*shm) == -1)
    {
	perror("close");
	return -1;
    }

    free(shm);

    return 0;
}
