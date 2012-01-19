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
 * \return
 */
GVlockptr
gvCreateLock(void);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] lock
 * \return
 */
int
gvAcquire(GVlockptr lock);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] lock
 * \return
 */
int
gvTryToAcquire(GVlockptr lock);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] lock 
 * \return
 */
int
gvRelease(GVlockptr lock);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] lock
 * \return
 */
int
gvDestroyLock(GVlockptr lock);

#endif /* LOCK_H */
