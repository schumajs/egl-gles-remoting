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

struct GVcontextstate {
    /* Context state */
    int            markedCurrent;
    int            markedDestroyed;

    /* Context transport */
    GVtransportptr transport;
};

typedef struct GVcontextstate *GVcontextstateptr;

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] display
 * \param  [in] context
 * \return
 */
int
gvDelEglContextState(EGLDisplay display,
		     EGLContext context);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] display
 * \param  [in] context
 * \return
 */
GVcontextstateptr
gvGetEglContextState(EGLDisplay display,
		     EGLContext context);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] display
 * \param  [in] context
 * \param  [in] state
 * \return
 */
int
gvSetEglContextState(EGLDisplay        display,
		     EGLContext        context,
		     GVcontextstateptr state);

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
int gvSetMarkCurrent(EGLDisplay display,
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
gvSetMarkDestroyed(EGLDisplay display,
		   EGLContext context);

/*! ***************************************************************************
 * \brief
 *
 * \return
 */
int
gvDelThreadTransport(void);

/*! ***************************************************************************
 * \brief
 *
 * \return
 */
GVtransportptr
gvGetThreadTransport(void);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] transport
 * \return
 */
int
gvSetThreadTransport(GVtransportptr transport);

#endif /* CLIENT_STATE_TRACKER_H_*/
