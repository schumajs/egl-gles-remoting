/*! ***************************************************************************
 * \file    heap_manager.h
 * \brief   
 * 
 * \date    December 25, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef SERVER_HEAP_MANAGER_H_
#define SERVER_HEAP_MANAGER_H_

#include <unistd.h>

#include "shared_memory.h"

#include "../heap_manager.h"

struct GVheapmgr {
    GVshmptr vmShm;
    GVshmptr heapMgrShm;
};

typedef struct GVheapmgr *GVheapmgrptr;

/*! ***************************************************************************
 * \brief 
 *
 * \param  [out] newHeapMgr
 * \param  [in]  heapSize
 * \return 
 */
int gvStartHeapMgr(GVheapmgrptr *newHeapMgr, size_t heapSize);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] heapMgr
 * \return 
 */
int gvStopHeapMgr(GVheapmgrptr heapMgr);

#endif /* SERVER_HEAP_MANAGER_H_ */
