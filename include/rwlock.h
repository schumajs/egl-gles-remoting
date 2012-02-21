/*! ***************************************************************************
 * \file    rwlock.h
 * \brief   
 * 
 * \date    December 20, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef RWLOCK_H_
#define RWLOCK_H_

#include <pthread.h>

typedef pthread_rwlock_t GVrwlock; 
typedef GVrwlock *GVrwlockptr;

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] desiredAddr
 * \return
 */
GVrwlockptr
gvCreateRwLock(void *desiredAddr);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] lock
 * \return
 */
int
gvAcquireReadLock(GVrwlockptr lock);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] lock
 * \return
 */
int
gvAcquireWriteLock(GVrwlockptr lock);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] lock 
 * \return
 */
int
gvReleaseRwLock(GVrwlockptr lock);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] lock
 * \return
 */
int
gvDestroyRwLock(GVrwlockptr lock);

#endif /* RWLOCK_H */
