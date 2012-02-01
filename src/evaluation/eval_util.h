/*! ***************************************************************************
 * \file    eval_util.h
 * \brief   
 * 
 * \date    January 31, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#ifndef EVAL_UTIL_H_
#define EVAL_UTIL_H_

#include <EGL/egl.h>

/* platform-specific */
typedef struct GVdisplay *GVdisplayptr;
typedef struct GVwindow  *GVwindowptr;

typedef int (*GVrenderfunc)(void);

/*! ***************************************************************************
 * \brief 
 *
 * \return
 */
GVdisplayptr
createDisplay(void);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] display
 * \param  [in] winTitle
 * \param  [in] winWidth
 * \param  [in] winHeight
 * \return
 */
GVwindowptr
createWindow(GVdisplayptr  display,
	     const char   *winTitle,
	     int           winWidth,
	     int           winHeight);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] display
 * \param  [in] window
 * \return
 */
EGLContext
createEglContext(GVdisplayptr display,
		 GVwindowptr  window);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] display
 * \param  [in] window
 * \param  [in] context
 * \param  [in] renderFunc
 * \return
 */
int
renderLoop(GVdisplayptr display,
	   GVwindowptr  window,
	   EGLContext   context,
           GVrenderfunc renderFunc);

#endif /* EVAL_UTIL_H_ */
