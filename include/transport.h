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

/*! ***************************************************************************
 * \namespace gvtrp
 */

#ifndef TRANSPORT_H_
#define TRANSPORT_H_

#include "lock.h"
#include "shared_memory.h"

struct GVTRPbuffer {
    GVLCKlockptr clientLock;
    GVLCKlockptr serverLock;
};

typedef struct GVTRPbuffer *GVTRPbufferptr;

struct GVTRPtransport {
    GVSHMshmptr    shm;
    size_t         offset;
    size_t         length;

    GVTRPbufferptr callBuffer;
    GVTRPbufferptr returnBuffer;
};

typedef struct GVTRPtransport *GVTRPtransportptr;

/*! ***************************************************************************
 * \brief 
 *
 * \param  [out] newTransport
 * \param  [in]  shm
 * \param  [in]  offset
 * \param  [in]  length
 * \return 
 */
int gvtrpCreate(GVTRPtransportptr *newTransport,
		GVSHMshmptr shm, size_t offset, size_t length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in]  buffer
 * \param  [out] spacePtr
 * \return 
 */
int gvtrpSpacePtr(GVTRPbufferptr buffer, void **spacePtr);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in]  buffer
 * \param  [out] length
 * \return 
 */
int gvtrpSpaceLength(GVTRPbufferptr buffer, size_t *length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] buffer
 * \param  [in] length
 * \return 
 */
int gvtrpGive(GVTRPbufferptr buffer, size_t length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] buffer
 * \param  [in] addr
 * \param  [in] length
 * \return 
 */
int gvtrpRead(GVTRPbufferptr buffer, void *addr, size_t length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in]  buffer
 * \param  [out] dataPtr
 * \return 
 */
int gvtrpDataPtr(GVTRPbufferptr buffer, void **dataPtr);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in]  buffer
 * \param  [out] length
 * \return 
 */
int gvtrpDataLength(GVTRPbufferptr buffer, size_t *length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] buffer
 * \param  [in] length
 * \return 
 */
int gvtrpTake(GVTRPbufferptr buffer, size_t length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] buffer
 * \param  [in] addr
 * \param  [in] length
 * \return 
 */
int gvtrpWrite(GVTRPbufferptr buffer, const void *addr, size_t length);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] transport
 * \return 
 */
int gvtrpDestroy(GVTRPtransportptr transport);

#endif /* TRANSPORT_H */
