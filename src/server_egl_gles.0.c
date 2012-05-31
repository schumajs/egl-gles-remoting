/*! ***************************************************************************
 * \file    server_egl_gles.0.c
 * \brief   
 * 
 * \date    January 9, 2012
 * \author  Jens Schumann
 *          schumajs@googlemail.com
 *
 * \details
 */

#include <stdlib.h>
#include <string.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "error.h"
#include "server_dispatcher.h"
#include "server_serializer.h"
#include "server_state_tracker.0.h"

static size_t
getSizeByType(GLenum type)
{
    size_t size;

    switch (type)
    {
    case GL_BYTE:
        size = sizeof(GLbyte);
        break;
    case GL_UNSIGNED_BYTE:
        size = sizeof(GLubyte);
        break;
    case GL_SHORT:
        size = sizeof(GLshort);
        break;
    case GL_UNSIGNED_SHORT:
        size = sizeof(GLushort);
        break;
    case GL_FIXED:
        size = sizeof(GLfixed);
        break;
    case GL_FLOAT:
        size = sizeof(GLfloat);
        break;
    }

    return size;
}

static int
receiveVertexAttribArrays(void **arrays[],
			  int   *numArrays)
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    int        numAttribs;
    GLsizei    numVertices;

    GLuint     index;
    GLint      size;
    GLenum     type;
    GLboolean  normalized;
    GLsizei    stride;
    void      *ptr;

    /* Receive number of vertex attributes */
    gvReceiveData(transport, &numAttribs, sizeof(int));

    /* Receive number of vertices */
    gvReceiveData(transport, &numVertices, sizeof(GLsizei));

    /* Receive vertex buffer indicator */
    int anyBufferBound;
    gvReceiveData(transport, &anyBufferBound, sizeof(int));

    /* Remember the vertex attribute arrays to be able to freeing them later in
     * _glDrawArrays and _glDrawElements
     */
    *arrays    = malloc(numAttribs * sizeof(void*));
    *numArrays = numAttribs;

    memset(*arrays, 0 , numAttribs * sizeof(void*));

    int     i;
    GLsizei blockSize;
    for (i = 0; i < numAttribs; i++)
    {
        /* Receive vertex attribute */
        gvReceiveData(transport, &index, sizeof(GLuint));
        gvReceiveData(transport, &size, sizeof(GLint));
        gvReceiveData(transport, &type, sizeof(GLenum));
        gvReceiveData(transport, &normalized, sizeof(GLboolean));
        gvReceiveData(transport, &stride, sizeof(GLsizei));
	gvReceiveData(transport, &ptr, sizeof(GLvoid*));

	if (!anyBufferBound)
	{
	    blockSize = numVertices * size * getSizeByType(type);
	
	    stride = 0;

	    ptr = malloc(blockSize);
	    gvReceiveData(transport, ptr, blockSize);
	}

        /* Specify and enable vertex attribute array */
        glVertexAttribPointer(index,
                              size,
                              type,
                              normalized,
                              stride,
                              ptr);

        glEnableVertexAttribArray(index);

	(*arrays)[i] = ptr;
    }

    return anyBufferBound;
}

static void
_notInUse()
{

}

/* ***************************************************************************
 * EGL
 */

static void
_eglGetError()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    EGLint         error;

    gvReceiveData(transport, &callId, sizeof(GVcallid));

    error = eglGetError();

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &error, sizeof(EGLint));
}

static void
_eglGetDisplay()
{
    GVtransportptr       transport = gvGetCurrentThreadTransport();

    GVcallid             callId;
    EGLDisplay           display;
    EGLNativeDisplayType displayId;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &displayId, sizeof(EGLNativeDisplayType));

    display = eglGetDisplay(displayId);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &display, sizeof(EGLDisplay));
}

static void
_eglInitialize()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();
    
    GVcallid       callId;
    EGLBoolean     status;
    EGLDisplay     display;
    EGLint         minor;
    EGLint         major;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));

    status = eglInitialize(display, &minor, &major);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &status, sizeof(EGLBoolean));
    gvSendData(transport, &major, sizeof(EGLint));
    gvSendData(transport, &minor, sizeof(EGLint));
}

static void
_eglTerminate()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    EGLBoolean     status;
    EGLDisplay     display;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));

    status = eglTerminate(display);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &status, sizeof(EGLBoolean));
}

static void
_eglQueryString()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    EGLDisplay      display;
    EGLint          name;
    const char     *queryString;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    gvReceiveData(transport, &name, sizeof(EGLint));

    queryString = eglQueryString(display, name);

    gvStartSending(transport, NULL, callId);
    gvSendVarSizeData(transport,
                      queryString,
                      (strlen(queryString) + 1) * sizeof(char));
}

static void
_eglGetConfigs()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    EGLBoolean      status;
    EGLDisplay      display;
    EGLConfig      *configs;
    EGLint          configSize;
    EGLint          numConfig;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    gvReceiveData(transport, &configSize, sizeof(EGLint));
    
    /* A - optional OUT pointer */
    if (configSize > 0)
    {
        configs = malloc(configSize);
    }
    else
    {
        configs = NULL;
    }
    
    status = eglGetConfigs(display, configs, configSize, &numConfig);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &status, sizeof(EGLBoolean));    
    gvSendData(transport, &numConfig, sizeof(EGLint));

    /* B - optional OUT pointer */
    if (configSize > 0)
    {
        gvSendData(transport, configs, numConfig * sizeof(EGLConfig));  
        free(configs);
    }
}

static void
_eglChooseConfig()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    EGLBoolean      status;
    EGLDisplay      display;
    EGLint         *attribList;
    EGLConfig      *configs;
    EGLint          configSize;
    EGLint          numConfig = 0;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    attribList = (EGLint *)gvReceiveVarSizeData(transport);
    gvReceiveData(transport, &configSize, sizeof(EGLint));

    /* A - optional OUT pointer */
    if (configSize > 0)
    {
        configs = malloc(configSize * sizeof(EGLConfig));
    }
    else
    {
        configs = NULL;
    }

    status = eglChooseConfig(display, attribList, configs, configSize, &numConfig);

    free(attribList);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &status, sizeof(EGLBoolean));
    gvSendData(transport, &numConfig, sizeof(EGLint));

    /* B - optional OUT pointer */
    if (configSize > 0 && numConfig > 0)
    {
        gvSendData(transport, configs, numConfig * sizeof(EGLConfig));
        free(configs);
    }
}

static void
_eglGetConfigAttrib()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    EGLBoolean      status;
    EGLDisplay      display;
    EGLConfig       config;
    EGLint          attribute;
    EGLint          value;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    gvReceiveData(transport, &config, sizeof(EGLConfig));
    gvReceiveData(transport, &attribute, sizeof(EGLint));

    status = eglGetConfigAttrib(display, config, attribute, &value);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &status, sizeof(EGLBoolean));
    gvSendData(transport, &value, sizeof(EGLint));
}

static void
_eglCreateWindowSurface()
{
    GVtransportptr       transport = gvGetCurrentThreadTransport();

    GVcallid             callId;
    EGLSurface           surface;
    EGLDisplay           display;
    EGLConfig            config;
    EGLNativeWindowType  window;
    EGLint              *attribList;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    gvReceiveData(transport, &config, sizeof(EGLConfig));
    gvReceiveData(transport, &window, sizeof(EGLNativeWindowType));
    attribList = (EGLint *)gvReceiveVarSizeData(transport);

    surface = eglCreateWindowSurface(display, config, window, attribList);

    free(attribList);
    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &surface, sizeof(EGLSurface));
}

static void
_eglCreatePbufferSurface()
{

}

static void
_eglCreatePixmapSurface()
{

}

static void
_eglDestroySurface()
{

}

static void
_eglQuerySurface()
{

}

static void
_eglBindAPI()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    EGLBoolean      status;
    EGLenum         api;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &api, sizeof(EGLenum));

    status = eglBindAPI(api);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &status, sizeof(EGLBoolean));
}

static void
_eglQueryAPI()
{

}

static void
_eglWaitClient()
{

}

static void
_eglReleaseThread()
{

}

static void
_eglCreatePbufferFromClientBuffer()
{

}

static void
_eglSurfaceAttrib()
{

}

static void
_eglBindTexImage()
{

}

static void
_eglReleaseTexImage()
{

}

static void
_eglSwapInterval()
{

}

static void
_eglCreateContext()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    EGLContext      context;
    EGLDisplay      display;
    EGLConfig       config;
    EGLContext      shareContext;
    EGLint         *attribList;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    gvReceiveData(transport, &config, sizeof(EGLConfig));
    gvReceiveData(transport, &shareContext, sizeof(EGLContext));
    attribList = (EGLint *)gvReceiveVarSizeData(transport);

    context = eglCreateContext(display, config, shareContext, attribList);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &context, sizeof(EGLContext));
}

static void
_eglDestroyContext()
{

}

static void
_eglMakeCurrent()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    EGLBoolean      status;
    EGLDisplay      display;
    EGLSurface      drawSurface;
    EGLSurface      readSurface;
    EGLContext      context;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    gvReceiveData(transport, &drawSurface, sizeof(EGLSurface));
    gvReceiveData(transport, &readSurface, sizeof(EGLSurface));
    gvReceiveData(transport, &context, sizeof(EGLContext));

    status = eglMakeCurrent(display, drawSurface, readSurface, context);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &status, sizeof(EGLBoolean));
}

static void
_eglGetCurrentContext()
{

}

static void
_eglGetCurrentSurface()
{

}

static void
_eglGetCurrentDisplay()
{

}

static void
_eglQueryContext()
{

}

static void
_eglWaitGL()
{

}

static void
_eglWaitNative()
{

}

static void
_eglSwapBuffers()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    EGLBoolean      status;
    EGLDisplay      display;
    EGLSurface      surface;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &display, sizeof(EGLDisplay));
    gvReceiveData(transport, &surface, sizeof(EGLSurface));
    
    status = eglSwapBuffers(display, surface);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &status, sizeof(EGLBoolean));
}

static void
_eglCopyBuffers()
{

}

/* ***************************************************************************
 * GLES2
 */

static void
_glCreateShader()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         shader;
    GLenum         type;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &type, sizeof(GLenum));

    shader = glCreateShader(type);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &shader, sizeof(GLuint));
}

static void
_glShaderSource()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         shader;
    GLsizei        count;
    GLchar       **string;
    GLint         *length;
    
    int             passingType;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &shader, sizeof(GLuint));
    gvReceiveData(transport, &count, sizeof(GLsizei));

    string = malloc(count * sizeof(GLchar*));

    /* There are several possibilities to pass "string", see spec. p. 27 */
    gvReceiveData(transport, &passingType, sizeof(int));

    if (passingType == 0)
    {
        length = NULL;
    }
    else if (passingType == 1)
    {
        length = gvReceiveVarSizeData(transport);
    }

    int i;
    for (i = 0; i < count; i++)
    {
        string[i] = gvReceiveVarSizeData(transport);
    }

    glShaderSource(shader, count, string, length);

    if (passingType == 0)
    {

    }
    else if (passingType == 1)
    {
        free(length);
    }

    for (i = 0; i < count; i++)
    {
        free(string[i]);
    }

    free(string);
}

static void
_glCompileShader()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         shader;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &shader, sizeof(GLuint));

    glCompileShader(shader);
}

static void
_glGetShaderiv()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         shader;
    GLenum         pname;
    GLint          params;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &shader, sizeof(GLuint));
    gvReceiveData(transport, &pname, sizeof(GLenum));

    glGetShaderiv(shader, pname, &params);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &params, sizeof(GLint));
}

static void
_glGetShaderInfoLog()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         shader;
    GLsizei        bufsize;
    GLsizei        length;
    GLchar        *infoLog;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &shader, sizeof(GLuint));
    gvReceiveData(transport, &bufsize, sizeof(GLsizei));

    infoLog = malloc(bufsize * sizeof(GLchar));

    glGetShaderInfoLog(shader, bufsize, &length, infoLog);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &length, sizeof(GLsizei));
    gvSendData(transport, infoLog, length * sizeof(GLchar));
}

static void
_glDeleteShader()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         shader;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &shader, sizeof(GLuint));

    glDeleteShader(shader);
}

static void
_glCreateProgram()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         program;

    gvReceiveData(transport, &callId, sizeof(GVcallid));

    program = glCreateProgram();

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &program, sizeof(GLuint));
}

static void
_glAttachShader()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         program;
    GLuint         shader;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &program, sizeof(GLuint));
    gvReceiveData(transport, &shader, sizeof(GLuint));

    glAttachShader(program, shader);
}

static void
_glBindAttribLocation()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    GLuint          program;
    GLuint          index;
    GLchar         *name;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &program, sizeof(GLuint));
    gvReceiveData(transport, &index, sizeof(GLuint));
    name = (GLchar *)gvReceiveVarSizeData(transport);

    glBindAttribLocation(program, index, name);

    free(name);
}

static void
_glLinkProgram()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         program;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &program, sizeof(GLuint));

    glLinkProgram(program);
}

static void
_glGetProgramiv()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         program;
    GLenum         pname;
    GLint          params;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &program, sizeof(GLuint));
    gvReceiveData(transport, &pname, sizeof(GLenum));

    glGetProgramiv(program, pname, &params);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &params, sizeof(GLint));
}

static void
_glGetProgramInfoLog()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         program;
    GLsizei        bufsize;
    GLsizei        length;
    GLchar        *infoLog;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &program, sizeof(GLuint));
    gvReceiveData(transport, &bufsize, sizeof(GLsizei));

    infoLog = malloc(bufsize * sizeof(GLchar));

    glGetShaderInfoLog(program, bufsize, &length, infoLog);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &length, sizeof(GLsizei));
    gvSendData(transport, infoLog, length * sizeof(GLchar));
}

static void
_glDeleteProgram()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         program;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &program, sizeof(GLuint));

    glDeleteProgram(program);
}

static void
_glClearColor()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLclampf       red;
    GLclampf       green;
    GLclampf       blue;
    GLclampf       alpha;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &red, sizeof(GLclampf));
    gvReceiveData(transport, &green, sizeof(GLclampf));
    gvReceiveData(transport, &blue, sizeof(GLclampf));
    gvReceiveData(transport, &alpha, sizeof(GLclampf));

    glClearColor(red, green, blue, alpha);
}

static void
_glViewport()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLint          x;
    GLint          y;
    GLsizei        width;
    GLsizei        height;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &x, sizeof(GLint));
    gvReceiveData(transport, &y, sizeof(GLint));
    gvReceiveData(transport, &width, sizeof(GLsizei));
    gvReceiveData(transport, &height, sizeof(GLsizei));

    glViewport(x, y, width, height);
}

static void
_glClear()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLbitfield     mask;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &mask, sizeof(GLbitfield));

    glClear(mask);
}

static void
_glUseProgram()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLuint         program;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &program, sizeof(GLuint));

    glUseProgram(program);   
}

static void
_glVertexAttribPointer()
{

}

static void
_glEnableVertexAttribArray()
{

}

static void
_glDrawArrays()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    GLenum          mode;
    GLint           first;
    GLsizei         count;

    void          **arrays;
    int             numArrays;

    int             anyBuffersBound;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &mode, sizeof(GLenum));
    gvReceiveData(transport, &first, sizeof(GLint));
    gvReceiveData(transport, &count, sizeof(GLsizei));

    anyBuffersBound = receiveVertexAttribArrays(&arrays, &numArrays);

    if (anyBuffersBound)
    {
	glDrawArrays(mode, first, count);
    } 
    else
    {
	glDrawArrays(mode, 0, count);

	int i;
	for (i = 0; i < numArrays; i++)
	{
	    if (arrays[i] != NULL)
	    {
		free(arrays[i]);
	    }
	}
    }

    free(arrays);
}

static void
_glGetError()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLenum         error;

    gvReceiveData(transport, &callId, sizeof(GVcallid));

    error = glGetError();

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &error, sizeof(GLenum));
}

static void
_glFinish()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;

    gvReceiveData(transport, &callId, sizeof(GVcallid));

    glFinish();

    gvStartSending(transport, NULL, callId);
}

static void
_glPixelStorei()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLenum         pname;
    GLint          param;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &pname, sizeof(GLenum));
    gvReceiveData(transport, &param, sizeof(GLint));

    glPixelStorei(pname, param);
}

static void
_glGenTextures()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    GLsizei         n;
    GLuint         *textures;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &n, sizeof(GLsizei));

    textures = malloc(n * sizeof(GLuint));

    glGenTextures(n, textures);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, textures, n * sizeof(GLuint));

    free(textures);
}

static void
_glBindTexture()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    GLenum          target;
    GLuint          texture;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &target, sizeof(GLenum));
    gvReceiveData(transport, &texture, sizeof(GLuint));

    glBindTexture(target, texture);
}

static void
_glTexImage2D()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    GLenum          target;
    GLint           level;
    GLint           internalFormat;
    GLsizei         width;
    GLsizei         height;
    GLint           border;
    GLenum          format;
    GLenum          type;
    GLvoid         *data;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &target, sizeof(GLenum));
    gvReceiveData(transport, &level, sizeof(GLint));
    gvReceiveData(transport, &internalFormat, sizeof(GLint));
    gvReceiveData(transport, &width, sizeof(GLsizei));
    gvReceiveData(transport, &height, sizeof(GLsizei));
    gvReceiveData(transport, &border, sizeof(GLint));
    gvReceiveData(transport, &format, sizeof(GLenum));
    gvReceiveData(transport, &type, sizeof(GLenum));

    data = (GLvoid *)gvReceiveVarSizeData(transport);

    glTexImage2D(target, level, internalFormat, width, height, border, format,
                 type, data);

    free(data);
}

static void
_glTexParameteri()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLenum         target;
    GLenum         pname;
    GLint          param;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &target, sizeof(GLenum));
    gvReceiveData(transport, &pname, sizeof(GLenum));
    gvReceiveData(transport, &param, sizeof(GLint));

    glTexParameteri(target, pname, param);
}

static void
_glActiveTexture()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLenum         texture;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &texture, sizeof(GLenum));

    glActiveTexture(texture);
}

static void
_glUniform1i()
{
    GVtransportptr transport = gvGetCurrentThreadTransport();

    GVcallid       callId;
    GLint          location;
    GLint          v0;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &location, sizeof(GLint));
    gvReceiveData(transport, &v0, sizeof(GLint));

    glUniform1i(location, v0);
}

static void
_glDrawElements()
{
    GVtransportptr   transport = gvGetCurrentThreadTransport();

    GVcallid         callId;
    GLenum           mode;
    GLsizei          count;
    GLenum           type;
    GLvoid          *indices;

    void           **arrays;
    int              numArrays;

    int              anyBuffersBound;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &mode, sizeof(GLenum));
    gvReceiveData(transport, &count, sizeof(GLsizei));
    gvReceiveData(transport, &type, sizeof(GLenum));

    indices = gvReceiveVarSizeData(transport);

    anyBuffersBound = receiveVertexAttribArrays(&arrays, &numArrays);

    glDrawElements(mode, count, type, indices);

    if (anyBuffersBound)
    {
	int i;
	for (i = 0; i < numArrays; i++)
	{
	    free(arrays[i]);
	}
    }

    free(arrays);

    free(indices);
}

static void
_glGetAttribLocation()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    GLuint          program;
    GLchar         *name;
    GLint           location;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &program, sizeof(GLuint));

    name = (GLchar *)gvReceiveVarSizeData(transport);

    location = glGetAttribLocation(program, name);

    free(name);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &location, sizeof(GLint));
}

static void
_glGetUniformLocation()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    GLuint          program;
    GLchar         *name;
    GLint           location;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &program, sizeof(GLuint));

    name = (GLchar *)gvReceiveVarSizeData(transport);

    location = glGetUniformLocation(program, name);

    free(name);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, &location, sizeof(GLint));
}

static void
_glDeleteTextures()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    GLsizei         n;
    GLuint         *textures;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &n, sizeof(GLsizei));

    /* TODO replace with gvReceiveData */
    textures = (GLuint *)gvReceiveVarSizeData(transport);

    glDeleteTextures(n, textures);

    free(textures);
}

static void
_glGenBuffers()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    GLsizei         n;
    GLuint         *buffers;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &n, sizeof(GLsizei));

    buffers = malloc(n * sizeof(GLuint));

    glGenBuffers(n, buffers);

    gvStartSending(transport, NULL, callId);
    gvSendData(transport, buffers, n * sizeof(GLuint));

    free(buffers);
}

static void
_glBindBuffer()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    GLenum          target;
    GLuint          buffer;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &target, sizeof(GLenum));
    gvReceiveData(transport, &buffer, sizeof(GLuint));

    glBindBuffer(target, buffer);
}

void
_glBufferData()
{
    GVtransportptr  transport;

    GVcallid        callId;
    GLenum          target;
    GLsizeiptr      size;
    GLvoid         *data;
    GLenum          usage;

    transport = gvGetCurrentThreadTransport();

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &target, sizeof(GLenum));
    gvReceiveData(transport, &size, sizeof(GLsizeiptr));

    data = malloc(size);
    gvReceiveData(transport, data, size);

    gvReceiveData(transport, &usage, sizeof(GLenum));

    glBufferData(target, size, data, usage);

    free(data);
}

static void
_glDeleteBuffers()
{
    GVtransportptr  transport = gvGetCurrentThreadTransport();

    GVcallid        callId;
    GLsizei         n;
    GLuint         *buffers;

    gvReceiveData(transport, &callId, sizeof(GVcallid));
    gvReceiveData(transport, &n, sizeof(GLsizei));

    /* TODO replace with gvReceiveData */
    buffers = (GLuint *)gvReceiveVarSizeData(transport);

    glDeleteBuffers(n, buffers);

    free(buffers);
}

/* ****************************************************************************
 * Jump table
 */

GVdispatchfunc eglGlesJumpTable[80] = {
    _notInUse,
    _notInUse,
    _notInUse,
    _notInUse,
    _notInUse,
    _notInUse,
    _notInUse,
    _notInUse,
    _notInUse,
    _notInUse,
    _eglGetError,
    _eglGetDisplay,
    _eglInitialize,
    _eglTerminate,
    _eglQueryString,
    _eglGetConfigs,
    _eglChooseConfig,
    _eglGetConfigAttrib,
    _eglCreateWindowSurface,
    _eglCreatePbufferSurface,
    _eglCreatePixmapSurface,
    _eglDestroySurface,
    _eglQuerySurface,
    _eglBindAPI,
    _eglQueryAPI,
    _eglWaitClient,
    _eglReleaseThread,
    _eglCreatePbufferFromClientBuffer,
    _eglSurfaceAttrib,
    _eglBindTexImage,
    _eglReleaseTexImage,
    _eglSwapInterval,
    _eglCreateContext,
    _eglDestroyContext,
    _eglMakeCurrent,
    _eglGetCurrentContext,
    _eglGetCurrentSurface,
    _eglGetCurrentDisplay,
    _eglQueryContext,
    _eglWaitGL,
    _eglWaitNative,
    _eglSwapBuffers,
    _eglCopyBuffers,
    _glCreateShader,
    _glShaderSource,
    _glCompileShader,
    _glGetShaderiv,
    _glGetShaderInfoLog,
    _glDeleteShader,
    _glCreateProgram,
    _glAttachShader,
    _glBindAttribLocation,
    _glLinkProgram,
    _glGetProgramiv,
    _glGetProgramInfoLog,
    _glDeleteProgram,
    _glClearColor,
    _glViewport,
    _glClear,
    _glUseProgram,
    _glVertexAttribPointer,
    _glEnableVertexAttribArray,
    _glDrawArrays,
    _glGetError,
    _glFinish,
    _glPixelStorei,
    _glGenTextures,
    _glBindTexture,
    _glTexImage2D,
    _glTexParameteri,
    _glActiveTexture,
    _glUniform1i,
    _glDrawElements,
    _glGetAttribLocation,
    _glGetUniformLocation,
    _glDeleteTextures,
    _glGenBuffers,
    _glBindBuffer,
    _glBufferData,
    _glDeleteBuffers
};
