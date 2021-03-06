/*! ***************************************************************************
 * \file    lock.h
 * \brief   
 * 
 * \date    December 20, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef LOCK_H_
#define LOCK_H_

#include <pthread.h>

typedef pthread_mutex_t GVlock; 
typedef GVlock *GVlockptr;

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] desiredAddr
 * \return
 */
GVlockptr
gvCreateLock(void *desiredAddr);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] lock
 * \return
 */
int
gvAcquireLock(GVlockptr lock);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] lock 
 * \return
 */
int
gvReleaseLock(GVlockptr lock);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] lock
 * \return
 */
int
gvDestroyLock(GVlockptr lock);

#endif /* LOCK_H */
