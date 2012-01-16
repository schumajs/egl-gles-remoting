/*! ***************************************************************************
 * \file    server_state_tracker.h
 * \brief   
 * 
 * \date    January 12, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef SERVER_STATE_TRACKER_H_
#define SERVER_STATE_TRACKER_H_

#include "transport.h" 

/*! ***************************************************************************
 * \brief 
 *
 * \return 
 */
int
gvInitStateTracker();

/*! ***************************************************************************
 * \brief 
 *
 * \param  [out] transport
 * \param  [out] jumpTable
 * \return 
 */
int
gvGetCurrent(GVtransportptr *transport);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] transport
 * \param  [in] jumpTable
 * \return 
 */
int
gvSetCurrent(GVtransportptr transport);

/*! ***************************************************************************
 * \brief 
 *
 * \return 
 */
int
gvTerminateStateTracker();

#endif /* SERVER_STATE_TRACKER_H_*/
