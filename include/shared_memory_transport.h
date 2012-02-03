/*! ***************************************************************************
 * \file    shared_memory_transport.h
 * \brief   
 * 
 * \date    December 20, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef SHARED_MEMORY_TRANSPORT_H_
#define SHARED_MEMORY_TRANSPORT_H_

#include "transport.h"

struct GVshmtransport {
    struct GVtransport base;

    GVshmptr           shm;
    size_t             offset;
    size_t             length;
};

typedef struct GVshmtransport *GVshmtransportptr;

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] shm
 * \param  [in] offset
 * \param  [in] length
 * \return 
 */
GVshmtransportptr
gvCreateShmTransport(GVshmptr shm,
		     size_t   offset,
		     size_t   length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] transport
 * \return 
 */
int
gvDestroyShmTransport(GVshmtransportptr transport);


#endif /* SHARED_MEMORY_TRANSPORT_H */
