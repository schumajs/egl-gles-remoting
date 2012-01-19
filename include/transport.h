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
#include "shared_memory.h"

struct GVbuffer {
    GVlockptr clientLock;
    GVlockptr serverLock;
};

typedef struct GVbuffer *GVbufferptr;

struct GVtransport {
    GVshmptr    shm;
    size_t      offset;
    size_t      length;

    GVbufferptr callBuffer;
    GVbufferptr returnBuffer;
};

typedef struct GVtransport *GVtransportptr;

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] shm
 * \param  [in] offset
 * \param  [in] length
 * \return 
 */
GVtransportptr
gvCreateTransport(GVshmptr shm,
		  size_t   offset,
		  size_t   length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] buffer
  * \return 
 */
void
*gvSpacePtr(GVbufferptr buffer);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] buffer
 * \return 
 */
size_t
gvSpaceLength(GVbufferptr buffer);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] buffer
 * \param  [in] length
 * \return 
 */
int
gvGive(GVbufferptr buffer,
       size_t      length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] buffer
 * \param  [in] addr
 * \param  [in] length
 * \return 
 */
int
gvRead(GVbufferptr  buffer,
       void        *addr,
       size_t       length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] buffer
 * \return 
 */
void
*gvDataPtr(GVbufferptr buffer);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] buffer
 * \return 
 */
size_t
gvDataLength(GVbufferptr buffer);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] buffer
 * \param  [in] length
 * \return 
 */
int
gvTake(GVbufferptr buffer,
       size_t      length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] buffer
 * \param  [in] addr
 * \param  [in] length
 * \return 
 */
int
gvWrite(GVbufferptr  buffer,
	const void  *addr,
	size_t       length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] transport
 * \return 
 */
int
gvDestroyTransport(GVtransportptr transport);

#endif /* TRANSPORT_H */
