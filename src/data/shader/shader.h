#ifndef DATA_SHADER_H
#define DATA_SHADER_H

#include "../../../lib/glad/glad.h"
#include "../data.h"

// -- shaders

// compilation log size
#define SHADER_LOG 1024

// macro for OpenGL shader compilation errors
#define GL_COMPILE_ERR(shader)                                        \
	{                                                                 \
	    GLint success;                                                \
	    glGetShaderiv(                                                \
	        shader,                                                   \
	        GL_COMPILE_STATUS,                                        \
	        &success                                                  \
	    );                                                            \
	    if(!success) {                                                \
	        char infoLog[SHADER_LOG];                                 \
	        glGetShaderInfoLog(                                       \
	            shader,                                               \
	            SHADER_LOG,                                           \
	            NULL,                                                 \
	            infoLog                                               \
	        );                                                        \
	        printf("%s compilation failed:\n%s\n", #shader, infoLog); \
	        exit(1);                                                  \
	    }                                                             \
	}

// macro for OpenGL shader linking errors
#define GL_LINK_ERR(program)                                       \
	{                                                              \
	    GLint success;                                             \
	    glGetProgramiv(                                            \
	        program,                                               \
	        GL_LINK_STATUS,                                        \
	        &success                                               \
	    );                                                         \
	    if(!success) {                                             \
	        char infoLog[SHADER_LOG];                              \
	        glGetProgramInfoLog(                                   \
	            program,                                           \
	            SHADER_LOG,                                        \
	            NULL,                                              \
	            infoLog                                            \
	        );                                                     \
	        printf("%s linking failed:\n%s\n", #program, infoLog); \
	        exit(1);                                               \
	    }                                                          \
	}

// shader data
typedef struct {
	GLuint program;
} shader;

// shader table
extern dataTable shaderTable;

// debug prints shaders
void shaderPrint(void* dat);

// shader handler declarations (can't use macro, diff. shaderImport signature)
dataRef* shaderImport(const char* vert, const char* frag);
void shaderFree(void* dat);

#endif
