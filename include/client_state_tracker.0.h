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
    /* Context */
    EGLDisplay     display;
    EGLContext     context;

    /* Context state */
    int            markedCurrent;
    int            markedDestroyed;

    /* Context transport */
    GVtransportptr transport;
};

typedef struct GVcontextstate *GVcontextstateptr;

typedef void (*GVforeachcontextfunc)(GVcontextstateptr);

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
 * \param  [in] func
 * \return
 */
int
gvForeachEglContextState(EGLDisplay           display,
			 GVforeachcontextfunc func);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] display
 * \param  [in] context
 * \param  [in] state
 * \return
 */
int
gvPutEglContextState(EGLDisplay        display,
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
int gvSetMarkedCurrent(EGLDisplay display,
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
gvSetMarkedDestroyed(EGLDisplay display,
		     EGLContext context);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] display
 * \return
 */
int
gvSetAllMarkedDestroyed(EGLDisplay display);

/*! ***************************************************************************
 * \brief
 *
 * \return
 */
GVtransportptr
gvGetCurrentThreadTransport(void);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] transport
 * \return
 */
int
gvSetCurrentThreadTransport(GVtransportptr transport);

#endif /* CLIENT_STATE_TRACKER_H_*/
