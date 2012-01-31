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

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] winTitle
 * \param  [in] winWidth
 * \param  [in] winHeight
 * \return
 */
EGLNativeWindowType
createEglNativeWindow(const char *winTitle,
		      int         winWidth,
		      int         winHeight);

/*! ***************************************************************************
 * \brief 
 *
 * \return
 */
EGLDisplay
createEglDisplay(void);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] display
 * \return
 */
EGLConfig
createEglConfig(EGLDisplay display);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] display
 * \param  [in] config
 * \param  [in] nativeWin
 * \return
 */
EGLSurface
createEglWindowSurface(EGLDisplay          display,
		       EGLConfig           config,
		       EGLNativeWindowType nativeWin);

/*! ***************************************************************************
 * \brief 
 *
 * \param  [in] display
 * \param  [in] config
 * \return
 */
EGLContext
createEglContext(EGLDisplay display,
		 EGLConfig  config);

#endif /* EVAL_UTIL_H_ */
