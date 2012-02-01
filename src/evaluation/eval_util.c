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

#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "error.h"
#include "eval_util.h"

struct GVdisplay {
    EGLDisplay  eglDisplay;
    EGLConfig   eglDisplayConfig;
    Display    *xDisplay;
};

struct GVwindow {
    EGLNativeWindowType eglNativeWindowHandle;
    EGLSurface          eglWindow;
    Window              xWindow;
};

static int
interrupted(Display *xDisplay)
{
    int     interrupted = 0;
    XEvent  xEvent;

    while (XPending(xDisplay))
    {
        XNextEvent(xDisplay, &xEvent);
        if (xEvent.type == DestroyNotify)
	{
	    interrupted = 1;
	}
    }

    return interrupted;
}

GVdisplayptr
createDisplay()
{
    static const EGLint attribList[] = {
	EGL_RED_SIZE, 1,
	EGL_GREEN_SIZE, 1,
	EGL_BLUE_SIZE, 1,
	EGL_DEPTH_SIZE, 1,
	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	EGL_NONE
    };

    GVdisplayptr display;

    EGLConfig  config;
    EGLint     numConfigs;

    TRY
    {
	if ((display = malloc(sizeof(struct GVdisplay))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	if ((display->xDisplay = XOpenDisplay(NULL)) == NULL)
	{
	    THROW(e0, "XOpenDisplay");
	}

	if (!(display->eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY)))
	{
	    THROW(e0, "eglGetDisplay");
	}

	if (!eglInitialize(display->eglDisplay, NULL, NULL))
	{
	    THROW(e0, "eglInitialize");
	}

	if (!eglChooseConfig(display->eglDisplay, attribList,
			     &display->eglDisplayConfig, 1, &numConfigs))
	{
	    THROW(e0, "eglChooseConfig");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return display;
}

GVwindowptr
createWindow(GVdisplayptr  display,
	     const char   *winTitle,
	     int           winWidth,
	     int           winHeight)
{
    GVwindowptr            window;

    Window               xRootWindow;
    XSetWindowAttributes xWindowAttribs;

    xWindowAttribs.event_mask = ExposureMask | PointerMotionMask | KeyPressMask;

    TRY
    {
	if ((window = malloc(sizeof(struct GVwindow))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	window->xWindow = XCreateWindow(display->xDisplay,
					DefaultRootWindow(display->xDisplay),
					0, 0,
					winWidth, winHeight,
					0,
					CopyFromParent,
					InputOutput,
					CopyFromParent,
					CWEventMask,
					&xWindowAttribs);


	XMapWindow(display->xDisplay, window->xWindow);
	XStoreName(display->xDisplay, window->xWindow, winTitle);

	XFlush(display->xDisplay);

	window->eglNativeWindowHandle = (EGLNativeWindowType) window->xWindow;

	if ((window->eglWindow
	     = eglCreateWindowSurface(display->eglDisplay,
				      display->eglDisplayConfig,
				      window->eglNativeWindowHandle,
				      NULL)) == EGL_NO_SURFACE)
	{
	    THROW(e0, "eglCreateWindowSurface");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return window;
}

EGLContext
createEglContext(GVdisplayptr display)
{
    static const EGLint attribList[] = {
	EGL_CONTEXT_CLIENT_VERSION, 2,
	EGL_NONE
    };

    EGLContext context;

    TRY
    {
	if ((context = eglCreateContext(display->eglDisplay,
					display->eglDisplayConfig,
					EGL_NO_CONTEXT,
					attribList)) == EGL_NO_CONTEXT)
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

void
renderLoop(GVdisplayptr display,
	   GVwindowptr  window,
	   EGLContext   context,
           GVrenderfunc renderFunc)
{
    while(!interrupted(display->xDisplay))
    {
	renderFunc();
        eglSwapBuffers(display->eglDisplay, window->eglWindow);
    }
}
