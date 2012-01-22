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
#include <GLES2/gl2.h>

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

typedef void (*GVforeachcontextfunc)(GVcontextstateptr, void*);

struct GVvertexattrib {
    GLuint        index;
    GLint         size;
    GLenum        type;
    GLboolean     normalized;
    GLsizei       stride;
    const GLvoid *ptr;
};

typedef struct GVvertexattrib *GVvertexattribptr;

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
gvForeachEglContextState(EGLDisplay            display,
			 GVforeachcontextfunc  func,
                         void                 *arg);

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
 * \param  [in] markedCurrent
 * \return
 */
int gvSetMarkedCurrent(EGLDisplay display,
		       EGLContext context,
		       int        markedCurrent);

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
 * \param  [in] markedDestroyed
 * \return
 */
int
gvSetMarkedDestroyed(EGLDisplay display,
		     EGLContext context,
                     int        markedDestroyed);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] display
 * \param  [in] markedDestroyed
 * \return
 */
int
gvSetAllMarkedDestroyed(EGLDisplay display,
                        int        markedDestroyed);

/*! ***************************************************************************
 * \brief
 *
 * \return
 */
EGLDisplay
gvGetCurrentThreadDisplay(void);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] transport
 * \return
 */
int
gvSetCurrentThreadDisplay(EGLDisplay display);

/*! ***************************************************************************
 * \brief
 *
 * \return
 */
EGLContext
gvGetCurrentThreadContext(void);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] transport
 * \return
 */
int
gvSetCurrentThreadContext(EGLContext context);

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

/*! ***************************************************************************
 * \brief
 *
 * \return
 */
GVvertexattribptr
gvGetCurrentVertexAttrib(void);

/*! ***************************************************************************
 * \brief
 *
 * \param  [in] vertexAttrib
 * \return
 */
int
gvSetCurrentVertexAttrib(GVvertexattribptr vertexAttrib);

#endif /* CLIENT_STATE_TRACKER_H_*/
