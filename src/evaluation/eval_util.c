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
#include <string.h>

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
	EGL_RED_SIZE, 5,
	EGL_GREEN_SIZE, 6,
	EGL_BLUE_SIZE, 5,
	EGL_ALPHA_SIZE, EGL_DONT_CARE,
	EGL_DEPTH_SIZE, EGL_DONT_CARE,
	EGL_STENCIL_SIZE, EGL_DONT_CARE,
	EGL_SAMPLE_BUFFERS, 0,
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
    GVwindowptr          window;

    XEvent               xEvent;
    Window               xRootWindow;
    XSetWindowAttributes xWindowAttribs1;
    XSetWindowAttributes xWindowAttribs2;
    XWMHints             xWindowMgrHints;
    Atom                 xWindowMgrState;

    TRY
    {
	if ((window = malloc(sizeof(struct GVwindow))) == NULL)
	{
	    THROW(e0, "malloc");
	}

	xWindowAttribs1.event_mask = ExposureMask | PointerMotionMask | KeyPressMask;

	window->xWindow = XCreateWindow(display->xDisplay,
					DefaultRootWindow(display->xDisplay),
					0, 0,
					winWidth, winHeight,
					0,
					CopyFromParent,
					InputOutput,
					CopyFromParent,
					CWEventMask,
					&xWindowAttribs1);

	xWindowAttribs2.override_redirect = 0;
	XChangeWindowAttributes(display->xDisplay,
				window->xWindow,
				CWOverrideRedirect,
				&xWindowAttribs2);

	xWindowMgrHints.input = 1;
	xWindowMgrHints.flags = InputHint;
	XSetWMHints(display->xDisplay, window->xWindow, &xWindowMgrHints);

	XMapWindow(display->xDisplay, window->xWindow);
	XStoreName(display->xDisplay, window->xWindow, winTitle);

	xWindowMgrState = XInternAtom(display->xDisplay, "_NET_WM_STATE", 0);

	memset(&xEvent, 0, sizeof(xEvent));
	xEvent.type                 = ClientMessage;
	xEvent.xclient.window       = window->xWindow;
	xEvent.xclient.message_type = xWindowMgrState;
	xEvent.xclient.format       = 32;
	xEvent.xclient.data.l[0]    = 1;
	xEvent.xclient.data.l[1]    = 0;
	XSendEvent(display->xDisplay,
		   DefaultRootWindow(display->xDisplay),
		   0,
		   SubstructureNotifyMask,
		   &xEvent);

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
createEglContext(GVdisplayptr display,
		 GVwindowptr  window)
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

	if (!eglMakeCurrent(display->eglDisplay,
			    window->eglWindow,
			    window->eglWindow,
			    context))
	{
	    THROW(e0, "eglMakeCurrent");
	}
    }
    CATCH (e0)
    {
	return NULL;
    }

    return context;
}

int
renderLoop(GVdisplayptr display,
	   GVwindowptr  window,
	   EGLContext   context,
           GVrenderfunc renderFunc)
{
    while (!interrupted(display->xDisplay))
    {
	if (renderFunc() == -1)
	{
	    return -1;
	}

        eglSwapBuffers(display->eglDisplay, window->eglWindow);
    }

    return 0;
}
