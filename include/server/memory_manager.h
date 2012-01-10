/*! ***************************************************************************
 * \file    memory_manager.h
 * \brief   
 * 
 * \date    December 25, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef SERVER_MEMORY_MANAGER_H_
#define SERVER_MEMORY_MANAGER_H_

#include <unistd.h>

#include "../memory_manager.h"
#include "../shared_memory.h"

struct GVMMGRcontext {
    GVSHMshmptr vmShm;
    size_t      vmShmSize;

    GVSHMshmptr mmgrShm;
    size_t      mmgrShmSize;
};

typedef struct GVMMGRcontext *GVMMGRcontextptr;

/*! ***************************************************************************
 * \brief 
 *
 * \param  [out] newMmgr
 * \param  [in]  heapSize
 * \return 
 */
int gvmmgrCreate(GVMMGRcontextptr *newMmgr, size_t heapSize);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] mmgr
 * \return 
 */
int gvmmgrDestroy(GVMMGRcontextptr mmgr);

#endif /* SERVER_MEMORY_MANAGER_H_ */
