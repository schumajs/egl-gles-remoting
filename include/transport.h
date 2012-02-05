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
typedef void *(*GVreadptrfunc)(GVchanelptr);
typedef int (*GVtakefunc)(GVchanelptr, size_t);
typedef int (*GVwritefunc)(GVchanelptr, const void*, size_t);
typedef void *(*GVwriteptrfunc)(GVchanelptr);
typedef int (*GVgivefunc)(GVchanelptr, size_t);
typedef void *(*GVpeekfunc)(GVchanelptr, size_t);


struct GVtransport {
    GVchanelptr    oc;
    GVchanelptr    ic;

    GVreadfunc     read;
    GVreadptrfunc  readPtr;
    GVtakefunc     take;
    GVwritefunc    write;
    GVwriteptrfunc writePtr;
    GVgivefunc     give;
    GVpeekfunc     peek;
};

typedef struct GVtransport *GVtransportptr;

#endif /* TRANSPORT_H */
