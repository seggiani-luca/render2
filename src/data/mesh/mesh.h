#ifndef DATA_MESH_H
#define DATA_MESH_H

#include "../data.h"
#include "../../../lib/glad/glad.h"

// -- meshes 

// vertex data type (mirrors OpenGL)
typedef struct {
	float x, y, z;    // position
	float u, v;       // UV
	float nx, ny, nz; // normal
} vertex;

// mesh data type
typedef struct {
	// array of vertices
	vertex* verts;

	// number of vertices
	int vertCount;
	
	// OpenGL VAO
	GLuint vao;
	
	// OpenGL VBO
	GLuint vbo;
} mesh;

// print mesh info
void meshPrint(mesh* mesh);

// mesh handler declarations
DATA_TABLE_DECL(mesh)

#endif
