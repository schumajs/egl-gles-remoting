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

/*! ***************************************************************************
 * \namespace gvlck
 */

#ifndef LOCK_H_
#define LOCK_H_

#include <pthread.h>

typedef pthread_mutex_t GVLCKlock; 
typedef GVLCKlock *GVLCKlockptr;

/*! ***************************************************************************
 * \brief 
 *
 * \param  [out] newLock
 * \return
 */
int gvlckCreate(GVLCKlockptr *newLock);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] lock
 * \return
 */
int gvlckAcquire(GVLCKlockptr lock);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] lock
 * \return
 */
int gvlckTryToAcquire(GVLCKlockptr lock);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] lock 
 * \return
 */
int gvlckRelease(GVLCKlockptr lock);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] lock
 * \return
 */
int gvlckDestroy(GVLCKlockptr lock);

#endif /* LOCK_H */
