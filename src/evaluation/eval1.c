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

#include <stdlib.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

/* OpenGL ES Programming Guide */
#include "esUtil.h"

#include "error.h"
#include "eval_util.h"

static GLuint programObject;

int
init()
{
    GLbyte vShaderStr[] =  
	"attribute vec4 vPosition;    \n"
	"void main()                  \n"
	"{                            \n"
	"   gl_Position = vPosition;  \n"
	"}                            \n";
   
    GLbyte fShaderStr[] =  
	"precision mediump float;\n"\
	"void main()                                  \n"
	"{                                            \n"
	"  gl_FragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );\n"
	"}                                            \n";

    GLint  linked;
   
    if ((programObject = esLoadProgram(vShaderStr, fShaderStr)) == 0)
    {
	return -1;
    }

    // Bind vPosition to attribute 0   
    glBindAttribLocation(programObject, 0, "vPosition");

    // Link the program
    glLinkProgram(programObject);

    // Check the link status
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

    if (!linked) 
    {
	GLint infoLen = 0;

	glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);
      
	if (infoLen > 1)
	{
	    char* infoLog = malloc (sizeof(char) * infoLen );

	    glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
	    
	    printf("INFO LOG %s", infoLog);

	    free (infoLog);
	}

	glDeleteProgram(programObject);

	return -1;
    }

    glClearColor(0.0f, 1.0f, 0.0f, 0.0f);

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    return 0;
}

static int
renderFunc()
{
    GLfloat vVertices[] = {
	 0.0f,  0.5f, 0.0f,
	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f
    };
      
    // Set the viewport
    glViewport(0, 0, 320, 240);
   
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the program object
    glUseProgram(programObject);

    // Load the vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    return 0;
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

	if (!(window = createWindow(display, "Triangles (no VBO)", 320, 240)))
	{
	    THROW(e0, "createWindow");
	}

	if (!(context = createEglContext(display, window)))
	{
	    THROW(e0, "createEglContext");
	}

	if (init() == -1)
	{
	    THROW(e0, "init");
	}

	if (renderLoop(display, window, context, renderFunc) == -1)
	{
	    THROW(e0, "renderLoop");
	}
    }
    CATCH (e0)
    {
	return 2;
    }

    return 0;
}
