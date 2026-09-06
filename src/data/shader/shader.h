#ifndef DATA_SHADER_H
#define DATA_SHADER_H

#include "../../../lib/glad/glad.h"
#include "../data.h"

// -- logging

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

// -- uniforms

// shader uniforms
typedef enum {
	// transform matrices 
	MODEL,
	VIEW,
	PROJECTION,

	// camera info	
	CAMERA_POSITION,

	// sun info
	SUN_DIRECTION,
	SUN_COLOR,

	// atmosphere info
	AMBIENT_COLOR,
	AMBIENT_MAP,
	HAS_AMBIENT_MAP,

	// material info
	DIFFUSE_COLOR,
	DIFFUSE_MAP,
	SPECULAR_COLOR,
	SPECULAR_MAP,
	SHININESS,
	SHININESS_MAP,
	SUBSURFACE_COLOR,
	HAS_DIFFUSE_MAP,
	HAS_SPECULAR_MAP,
	HAS_SHININESS_MAP
} shaderUniform;

// number of shader uniforms
#define NUM_UNIFORMS (HAS_SHININESS_MAP + 1)

// types of shader uniforms
typedef enum {
	UNIFORM_1I,
	UNIFORM_1F,
	UNIFORM_3FV,
	UNIFORM_MAT4,
	UNIFORM_TEX
} uniformType;

// shader uniform info 
typedef struct {
	// name of uniform
	const char* name;

	// auxiliary uniform (presence flag for textures)
	shaderUniform auxil;

	// type of uniform
	uniformType type;
} uniformInfo;

// shader uniform info for all uniforms
static const uniformInfo uniformInfos[NUM_UNIFORMS] = {
	[MODEL] = {
		.name = "uModel",
		.type = UNIFORM_MAT4
	},
	[VIEW] = {
		.name = "uView",
		.type = UNIFORM_MAT4
	},
	[PROJECTION] = {
		.name = "uProjection",
		.type = UNIFORM_MAT4
	},
	[CAMERA_POSITION] = {
		.name = "uCameraPos",
		.type = UNIFORM_3FV
	},
	[SUN_DIRECTION] = {
		.name = "uSunDir",
		.type = UNIFORM_3FV
	},
	[SUN_COLOR] = {
		.name = "uSunCol",
		.type = UNIFORM_3FV
	},
	[AMBIENT_COLOR] = {
		.name = "uAmbientCol",
		.type = UNIFORM_3FV
	},
	[AMBIENT_MAP] = {
		.name = "uAmbientMap",
		.auxil = HAS_AMBIENT_MAP,
		.type = UNIFORM_TEX 
	},
	[HAS_AMBIENT_MAP] = {
		.name = "uHasAmbientMap",
		.type = UNIFORM_1I
	},
	[DIFFUSE_COLOR] = {
		.name = "uDiffuseCol",
		.type = UNIFORM_3FV
	},
	[DIFFUSE_MAP] = {
		.name = "uDiffuseMap",
		.auxil = HAS_DIFFUSE_MAP,
		.type = UNIFORM_TEX 
	},
	[SPECULAR_COLOR] = {
		.name = "uSpecularCol",
		.type = UNIFORM_3FV
	},
	[SPECULAR_MAP] = {
		.name = "uSpecularMap",
		.auxil = HAS_SPECULAR_MAP,
		.type = UNIFORM_TEX
	},
	[SHININESS] = {
		.name = "uShininess",
		.type = UNIFORM_1F
	},
	[SHININESS_MAP] = {
		.name = "uShininessMap",
		.auxil = HAS_SHININESS_MAP,
		.type = UNIFORM_TEX 
	},
	[SUBSURFACE_COLOR] = {
		.name = "uSubsurfCol",
		.type = UNIFORM_3FV
	},
	[HAS_DIFFUSE_MAP] = {
		.name = "uHasDiffuseMap",
		.type = UNIFORM_1I
	},
	[HAS_SPECULAR_MAP] = {
		.name = "uHasSpecularMap",
		.type = UNIFORM_1I
	},
	[HAS_SHININESS_MAP] = {
		.name = "uHasShininessMap",
		.type = UNIFORM_1I
	}
};

// shader data
typedef struct {
	GLuint program;
	GLint uniformLocations[NUM_UNIFORMS];
} shader;

// resets the texture units
void resetTexUnit();

// sends an uniform to the GPU
void sendUniform(shader* shader, shaderUniform uniform, const void* data);

// -- shaders

// shader table
extern dataTable shaderTable;

// debug prints shaders
void shaderPrint(void* dat);

// shader handler declarations (can't use macro, diff. shaderImport signature)
dataRef* shaderImport(const char* vert, const char* frag);
void shaderFree(void* dat);

#endif
