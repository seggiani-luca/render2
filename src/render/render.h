#ifndef RENDER_H
#define RENDER_H
#include <stdio.h>

// macro for OpenGL errors
#define GL_ERR(func)                                     \
	{                                                    \
	    GLenum err = glGetError();                       \
	    if(err != GL_NO_ERROR)                           \
	    printf("OpenGL error 0x%x at %s\n", err, #func); \
	}

#endif
