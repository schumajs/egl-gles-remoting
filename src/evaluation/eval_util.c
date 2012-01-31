/*! ***************************************************************************
 * \file    eval_util.c
 * \brief   
 * 
 * \date    January 31, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "error.h"
#include "eval_util.h"

EGLNativeWindowType
createEglNativeWindow(const char *winTitle,
		      int         winWidth,
		      int         winHeight)
{
    Display             *disp;
    Window               root;
    Window               win;
    XSetWindowAttributes winAttribs;

    TRY
    {
	if ((disp = XOpenDisplay(NULL)) == NULL)
	{
	    THROW(e0, "XOpenDisplay");
	}
    }
    CATCH (e0)
    {
	return -1;
    }

    root = DefaultRootWindow(disp);

    winAttribs.event_mask = ExposureMask | PointerMotionMask | KeyPressMask;

    win = XCreateWindow(disp, root,
			0, 0,
			winWidth, winHeight,
			0,
			CopyFromParent,
			InputOutput,
			CopyFromParent,
			CWEventMask,
			&winAttribs);

    XMapWindow(disp, win);
    XStoreName(disp, win, winTitle);
    XFlush(disp);

    return (EGLNativeWindowType) win;
}

EGLDisplay
createEglDisplay()
{
    EGLDisplay display;

    TRY
    {
	if (!(display = eglGetDisplay(EGL_DEFAULT_DISPLAY)))
	{
	    THROW(e0, "eglGetDisplay");
	}

	if (!eglInitialize(display, NULL, NULL))
	{
	    THROW(e0, "eglInitialize");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return display;
}


EGLConfig
createEglConfig(EGLDisplay display)
{
    static const EGLint attribList[] = {
	EGL_RED_SIZE, 1,
	EGL_GREEN_SIZE, 1,
	EGL_BLUE_SIZE, 1,
	EGL_DEPTH_SIZE, 1,
	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	EGL_NONE
    };

    EGLConfig config;
    EGLint    numConfigs;

    TRY
    {
	if (!eglChooseConfig(display, attribList, config, 1, &numConfigs))
	{
	    THROW(e0, "eglChooseConfig");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return config;
}

EGLSurface
createEglWindowSurface(EGLDisplay          display,
		       EGLConfig           config,
		       EGLNativeWindowType nativeWin)
{
    EGLSurface surface;

    TRY
    {
	if ((surface
	     = eglCreateWindowSurface(display, config,
				      nativeWin, NULL)) == EGL_NO_SURFACE)
	{
	    THROW(e0, "eglCreateWindowSurface");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return surface;
}

EGLContext
createEglContext(EGLDisplay display,
		 EGLConfig  config)
{
    static const EGLint attribList[] = {
	EGL_CONTEXT_CLIENT_VERSION, 2,
	EGL_NONE
    };

    EGLContext context;

    TRY
    {
	if ((context
	     = eglCreateContext(display, config,
				EGL_NO_CONTEXT, attribList)) == EGL_NO_CONTEXT)
	{
	    THROW(e0, "eglCreateContext");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return context;
}
