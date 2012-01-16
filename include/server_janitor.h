/*! ***************************************************************************
 * \file    server_janitor.h
 * \brief   
 * 
 * \date    December 25, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef SERVER_JANITOR_H_
#define SERVER_JANITOR_H_

#include "shared_memory.h"

#include "./janitor.h"

struct GVjanitor {
    GVshmptr vmShm;
    GVshmptr janitorShm;
};

typedef struct GVjanitor *GVjanitorptr;

/*! ***************************************************************************
 * \brief 
 *
 * \param  [out] newJanitor
 * \param  [in]  vmShm
 * \return 
 */
int
gvStartJanitor(GVjanitorptr *newJanitor, GVshmptr vmShm);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] server
 * \return 
 */
int
gvStopJanitor(GVjanitorptr server);

#endif /* SERVER_JANITOR_H_ */
