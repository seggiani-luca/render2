#ifndef RENDER_H
#define RENDER_H

#include "../math/math.h"
#include <stdio.h>

// -- constants

// rendering transform name
#define REN_TRANSFORM_NAME "Transform"

// rendering mesh name
#define REN_MESH_NAME "Mesh"

// rendering material name
#define REN_MATERIAL_NAME "Material"

// rendering camera name
#define REN_CAMERA_NAME "Camera"

// rendering atmosphere name
#define REN_ATMOSPHERE_NAME "Atmosphere"

// -- rendering

// paths for shaders
#define VERT_PATH "dat/shader/vert.glsl"
#define PHONG_FRAG_PATH "dat/shader/phong_frag.glsl"

// macro for OpenGL errors
#define GL_ERR(func)                                     \
	{                                                    \
	    GLenum err = glGetError();                       \
	    if(err != GL_NO_ERROR)                           \
	    printf("OpenGL error 0x%x at %s\n", err, #func); \
	}

// -- rendering objects

// definition of transform
typedef struct {
	// position
	float3 position;

	// rotation
	quat rotation;

	// scale
	float3 scale;
} transform;

// definition of camera 
typedef struct {
	// camera field of view
	float fov;

	// camera near plane
	float nearPlane;

	// camera far plane
	float farPlane;
} camera;

// definition of atmosphere 
typedef struct {
	// ambient color
	color ambient;
} atmosphere;

#endif
