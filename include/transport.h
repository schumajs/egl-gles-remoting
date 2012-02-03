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

struct GVstream {
    GVlockptr exclusiveAccess;
};

typedef struct GVstream *GVstreamptr;

typedef int (*GVreadfunc)(GVstreamptr, void*, size_t);
typedef int (*GVwritefunc)(GVstreamptr, const void*, size_t);
typedef void* (*GVpeekfunc)(GVstreamptr, size_t);
typedef int (*GVskipfunc)(GVstreamptr, size_t);

struct GVtransport {
    GVstreamptr is;
    GVstreamptr os;

    GVreadfunc  read;
    GVwritefunc write;
    GVpeekfunc  peek;
    GVskipfunc  skip;
};

typedef struct GVtransport *GVtransportptr;

#endif /* TRANSPORT_H */
