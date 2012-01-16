/*! ***************************************************************************
 * \file    client_state_tracker.h
 * \brief   
 * 
 * \date    January 12, 2011
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef CLIENT_STATE_TRACKER_H_
#define CLIENT_STATE_TRACKER_H_

#include <EGL/egl.h>

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
 * \param  [in] display
 * \param  [in] context
 * \return 
 */
int
gvTrack(EGLDisplay display,
	EGLContext context);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] display
 * \param  [in] context
 * \return 
 */
int
gvUntrack(EGLDisplay display,
	  EGLContext context);


/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] display
 * \param  [in] context
 * \return 
 */
int
gvIsMarkedCurrent(EGLDisplay display,
		  EGLContext context);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] display
 * \param  [in] context
 * \return 
 */
int
gvMarkCurrent(EGLDisplay display,
	      EGLContext context);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] display
 * \param  [in] context
 * \return 
 */
int
gvIsMarkedDestroyed(EGLDisplay display,
		    EGLContext context);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] display
 * \param  [in] context
 * \return 
 */
int
gvMarkDestroyed(EGLDisplay display,
		EGLContext context);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] display
 * \param  [in] context
 * \return 
 */
int
gvGetCurrent(EGLDisplay     *display,
	     EGLContext     *context,
	     GVtransportptr *transport);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] display
 * \param  [in] context
 * \return 
 */
int
gvSetCurrent(EGLDisplay     display,
	     EGLContext     context,
	     GVtransportptr transport);

/*! ***************************************************************************
 * \brief 
 *
 * \return 
 */
int
gvTerminateStateTracker();

#endif /* CLIENT_STATE_TRACKER_H_*/
