/*! ***************************************************************************
 * \file    server_state_tracker.0.h
 * \brief
 * 
 * \date    January 9, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef SERVER_STATE_TRACKER_0_H_
#define SERVER_STATE_TRACKER_0_H_

#include <pthread.h>
#include <unistd.h>

#include "transport.h"

/*! ***************************************************************************
 * \brief
 *
 * \param  [out] transport
 * \return
 */
int
gvGetCurrentThreadTransport(GVtransportptr *transport);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] transport
 * \return
 */
int
gvSetCurrentThreadTransport(GVtransportptr transport);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] offset
 * \return
 */
int
gvDelJanitorState(size_t offset);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in]  offset
 * \param  [out] thread
 * \param  [out] transport
 * \return
 */
int
gvGetJanitorState(size_t          offset,
		  pthread_t      *thread,
		  GVtransportptr *transport);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] offset
 * \param  [in] thread
 * \param  [in] transport
 * \return
 */
int
gvPutJanitorState(size_t         offset,
		  pthread_t      thread,
		  GVtransportptr transport);

#endif /* SERVER_STATE_TRACKER_0_H_ */
