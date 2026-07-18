#ifndef RENDER_H
#define RENDER_H

#include "../math/math.h"
#include "../data/texture/texture.h"
#include "../data/mesh/mesh.h"
#include "../data/material/material.h"
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

// -- objects

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

// -- views 

// an entity, which can be rendered to screen
struct renderEntity {
	// entity transformation
	mat4 transform;

	// mesh of entity
	mesh* mesh;
	
	// material of entity
	material* material;

	// next render entity in list
	struct renderEntity* next;
};
typedef struct renderEntity renderEntity;

// a rendering view of a scene
typedef struct {
	// scene camera
	struct {
		// camera field
		camera* info;

		// camera transform
		mat4 transform;
	} camera;

	// scene atmosphere 
	atmosphere* atmosphere;

	// root of scene hierarchy
	renderEntity* root;
} renderScene;

// forward declaration for updateRenderScene
typedef struct scene scene;

// updates rendering view of a scene
void updateRenderScene(scene* scene);

#endif
