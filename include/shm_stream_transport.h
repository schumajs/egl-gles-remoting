/*! ***************************************************************************
 * \file    shm_stream_transport.h
 * \brief   
 * 
 * \date    December 20, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef SHM_STREAM_TRANSPORT_H_
#define SHM_STREAM_TRANSPORT_H_

#include "shared_memory.h"
#include "transport.h"

struct GVchanel {
    GVlockptr exclusiveAccess;
};

struct GVshmstreamtrp {
    struct GVtransport base;

    GVshmptr           shm;
    size_t             offset;
    size_t             length;
};

typedef struct GVshmstreamtrp *GVshmstreamtrpptr;

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] shm
 * \param  [in] offset
 * \param  [in] length
 * \return 
 */
GVtransportptr
gvCreateShmStreamTransport(GVshmptr shm,
			   size_t   offset,
			   size_t   length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] transport
 * \return 
 */
int
gvDestroyShmStreamTransport(GVtransportptr transport);

#endif /* SHM_STREAM_TRANSPORT_H */
