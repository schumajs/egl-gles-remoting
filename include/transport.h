/*! ***************************************************************************
 * \file    transport.h
 * \brief   
 * 
 * \date    December 20, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef TRANSPORT_H_
#define TRANSPORT_H_

#include "lock.h"

typedef struct GVchanel *GVchanelptr;

typedef int (*GVreadfunc)(GVchanelptr, void*, size_t);
typedef int (*GVwritefunc)(GVchanelptr, const void*, size_t);
typedef void* (*GVpeekfunc)(GVchanelptr, size_t);
typedef int (*GVskipfunc)(GVchanelptr, size_t);

struct GVtransport {
    GVchanelptr oc;
    GVchanelptr ic;

    GVreadfunc  read;
    GVwritefunc write;
    GVpeekfunc  peek;
    GVskipfunc  skip;
};

typedef struct GVtransport *GVtransportptr;

#endif /* TRANSPORT_H */
