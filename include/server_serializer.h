/*! ***************************************************************************
 * \file    server_serializer.h
 * \brief
 * 
 * \date    December 20, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef SERVER_SERIALZIER_H
#define SERVER_SERIALIZER_H

#include "serializer.h"

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] transport
 * \param  [in] lock
 * \param  [in] callId
 */
GVcmdid
gvStartReceiving(GVtransportptr transport,
		 GVlockptr      lock);

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
 * \param  [in] lock
 * \param  [in] callId
 * \return
 */
int
gvStartSending(GVtransportptr transport,
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
gvStopSending(GVtransportptr transport,
	      GVlockptr      lock);

#endif /* SERVER_SERIALIZER_H */
