/*! ***************************************************************************
 * \file    eval1.c
 * \brief   Triangle strips (no VBO)
 * 
 * \date    January 31, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <EGL/egl.h>

#include "error.h"
#include "eval_util.h"

static void
render()
{
    
}

int
main(int   argc,
     char *argv[])
{
    GVdisplayptr display;
    GVwindowptr  window;
    EGLContext   context;

    TRY
    {
	if (!(display = createDisplay()))
	{
	    THROW(e0, "createDisplay");
	}

	if (!(window = createWindow(display, "Triangles (no VBO)", 640, 480)))
	{
	    THROW(e0, "createWindow");
	}

	if (!(context = createEglContext(display)))
	{
	    THROW(e0, "createEglContext");
	}

	renderLoop(display, window, context, render);
    }
    CATCH (e0)
    {
	return 2;
    }

    return 0;
}
