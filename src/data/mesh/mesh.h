#ifndef DATA_MESH_H
#define DATA_MESH_H

#include "../data.h"

// -- meshes 

// vertex data type (mirrors OpenGL)
typedef struct {
	float x, y, z;    // position
	float u, v;       // uvs
	float nx, ny, nz; // normal
} vertex;

// mesh data type
typedef struct {
} mesh;

// print mesh info
void texturePrint(mesh* mesh);

// mesh handler declaration
DATA_TABLE_DECL(mesh)

#endif
