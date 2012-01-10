/*! ***************************************************************************
 * \file    server.h
 * \brief   
 * 
 * \date    December 25, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#include "shared_memory.h"

#include "../server.h"

struct GVSRVcontext {
    GVSHMshmptr srvShm;
    size_t      srvShmSize;
};

typedef struct GVSRVcontext *GVSRVcontextptr;

/*! ***************************************************************************
 * \brief 
 *
 * \param  [out] newServer
 * \param  [in]  vmShm
 * \return 
 */
int gvsrvCreate(GVSRVcontextptr *newServer, GVSHMshmptr vmShm);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] server
 * \return 
 */
int gvsrvDestroy(GVSRVcontextptr server);

#endif /* SERVER_SERVER_H_ */
