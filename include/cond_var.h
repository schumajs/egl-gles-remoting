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

typedef pthread_cond_t GVcond;
typedef GVcond *GVcondptr;

struct GVcondvar {
    GVlockptr lock;
    GVcondptr cond;
};

typedef struct GVcondvar *GVcondvarptr;

/*! ***************************************************************************
 * \brief 
 *
 * \return
 */
GVcondvarptr
gvCreateCondVar(void);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] condVar
 * \return
 */
int
gvWait(GVcondvarptr condVar);

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
