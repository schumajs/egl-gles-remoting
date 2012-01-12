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

/*! ***************************************************************************
 * \namespace gvshm
 */

#ifndef SHARED_MEMORY_H_
#define SHARED_MEMORY_H_

#include <unistd.h>

typedef int GVSHMshm; 
typedef GVSHMshm *GVSHMshmptr;

/*! ***************************************************************************
 * \brief 
 *
 * \param  [out] newShm
 * \param  [in]  shmSize
 * \return 
 */
int gvshmCreate(GVSHMshmptr *newShm, size_t shmSize);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] toAddr
 * \param  [in] shm
 * \param  [in] offset
 * \param  [in] length
 * \return 
 */
int gvshmAttach(void *toAddr, GVSHMshmptr shm, off_t offset, size_t length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] fromAddr
 * \param  [in] shm
 * \param  [in] offset
 * \param  [in] length
 * \return 
 */
int gvshmDetach(void *fromAddr, GVSHMshmptr shm, off_t offset, size_t length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] shm
 * \return
 */
int gvshmDestroy(GVSHMshmptr shm);

#endif /* SHARED_MEMORY_H */
