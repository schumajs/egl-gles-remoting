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

#include "error.h"
#include "shared_memory.h"

GVshmptr
gvCreateShm(size_t shmSize)
{
    int      fd;
    char     fileName[] = "/dev/shm/gvshm.XXXXXX";
    GVshmptr newShm;	

    TRY
    {
	if ((fd = mkstemp(fileName)) == -1)
	{
	    THROW(e0, "mkstemp");
	}

	if (unlink(fileName) == -1)
	{
	    THROW(e0, "unlink");
	}

	if (ftruncate(fd, shmSize) == -1)
	{
	    THROW(e0, "ftruncate");
	}

	if ((newShm = malloc(sizeof(struct GVshm))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	newShm->id   = fd;
	newShm->size = shmSize;
    }
    CATCH (e0)
    {
	return NULL;
    }

    return newShm;
}

int
gvAttachShm(void *toAddr, GVshmptr shm, size_t offset, size_t length)
{
    TRY
    {
	if (mmap(toAddr, length, PROT_READ | PROT_WRITE,
		 MAP_FIXED | MAP_SHARED, shm->id, offset) == MAP_FAILED)
	{
	    THROW(e0, "mmap");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvDetachShm(void *fromAddr, GVshmptr shm, size_t offset, size_t length)
{
    TRY
    {
	if (munmap(fromAddr, length) == -1)
	{
	    THROW(e0, "munmap");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}

int
gvDestroyShm(GVshmptr shm)
{
    TRY
    {
	if (close(shm->id) == -1)
	{
	    THROW(e0, "close");
	}

	free(shm);
    }
    CATCH (e0)
    {
	return -1;
    }

    return 0;
}
