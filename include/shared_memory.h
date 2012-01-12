/*! ***************************************************************************
 * \file    shared_memory.h
 * \brief   
 * 
 * \date    December 20, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef SHARED_MEMORY_H_
#define SHARED_MEMORY_H_

#include <unistd.h>

typedef int GVshmid;

typedef struct GVshm {
    GVshmid id;
    size_t  size;
}
 
typedef GVshm *GVshmptr;

/*! ***************************************************************************
 * \brief 
 *
 * \param  [out] newShm
 * \param  [in]  shmSize
 * \return 
 */
int gvCreateShm(GVshmptr *newShm, size_t shmSize);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] toAddr
 * \param  [in] shm
 * \param  [in] offset
 * \param  [in] length
 * \return 
 */
int gvAttachShm(void *toAddr, GVshmptr shm, off_t offset, size_t length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] fromAddr
 * \param  [in] shm
 * \param  [in] offset
 * \param  [in] length
 * \return 
 */
int gvDetachShm(void *fromAddr, GVshmptr shm, off_t offset, size_t length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] shm
 * \return
 */
int gvDestroyShm(GVSHMshmptr shm);

#endif /* SHARED_MEMORY_H */
