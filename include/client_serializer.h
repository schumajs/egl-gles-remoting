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

#endif /* CLIENT_SERIALIZER_H */
