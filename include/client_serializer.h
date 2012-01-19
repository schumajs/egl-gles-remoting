/*! ***************************************************************************
 * \file    client_serializer.h
 * \brief
 * 
 * \date    December 20, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef CLIENT_SERIALZIER_H
#define CLIENT_SERIALIZER_H

#include "serializer.h"

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] transport
 * \param  [in] lock
 * \param  [in] cmdId
 * \return
 */
GVcallid
gvStartSending(GVtransportptr transport,
	       GVlockptr      lock,
	       GVcmdid        cmdId);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] transport
 * \param  [in] lock
 * \return
 */
int
gvStopSending(GVtransportptr transport,
	      GVlockptr      lock);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] transport
 * \param  [in] lock
 * \param  [in] callId
 */
int
gvStartReceiving(GVtransportptr transport,
		 GVlockptr      lock,
                 GVcallid       callId);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] transport
 * \param  [in] lock
 * \return
 */
int
gvStopReceiving(GVtransportptr transport,
		GVlockptr      lock);

/*! ***************************************************************************
 * \brief
 * 
 * \param  [in] transport
 * \param  [in] toAddr
 * \param  [in] length
 * \return
 */
int
gvReceiveData(GVtransportptr  transport,
	      void           *toAddr, 
	      size_t          length);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] transport
 * \param  [in] fromAddr
 * \param  [in] length
 * \return
 */
int
gvSendData(GVtransportptr  transport,
	   const void     *fromAddr,
	   size_t          length);

#endif /* CLIENT_SERIALIZER_H */
