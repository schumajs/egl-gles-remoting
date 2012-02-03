/*! ***************************************************************************
 * \file    cond_var.h
 * \brief   
 * 
 * \date    January 20, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef COND_VAR_H_
#define COND_VAR_H_

#include <pthread.h>

#include "lock.h"

typedef pthread_cond_t GVcondvar;
typedef GVcondvar *GVcondvarptr;

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] desiredAddr
 * \return
 */
GVcondvarptr
gvCreateCondVar(void *desiredAddr);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] condVar
 * \param  [in] lock
 * \return
 */
int
gvWait(GVcondvarptr condVar, GVlockptr lock);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] condVar
 * \return
 */
int
gvNotify(GVcondvarptr condVar);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] condVar
 * \return
 */
int
gvDestroyCondVar(GVcondvarptr condVar);

#endif /* COND_VAR_H */
