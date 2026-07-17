#include "mesh.h"
#include "../../render/render.h"
#include <stdlib.h>
#include <string.h>

extern int meshDecode(mesh* mesh, FILE* file);

void meshPrint(mesh* mesh) {
	printf("Mesh (vertices: %d)", mesh->vertCount);
}

// generates a VBO and a VAO for this mesh
void generateGLMeshes(mesh* mesh) {
	// generate VBO
	glGenBuffers(1, &mesh->vbo);
	GL_ERR("VBO generation");

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	GL_ERR("VBO binding");
	
	glBufferData(
		GL_ARRAY_BUFFER, 
		mesh->vertCount * sizeof(vertex), 
		mesh->verts,
		GL_STATIC_DRAW
	);
	GL_ERR("VBO data");

	// generate VAO
	glGenVertexArrays(1, &mesh->vao);
	GL_ERR("VAO generation");
	
	glBindVertexArray(mesh->vao);
	GL_ERR("VAO binding");
	
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	GL_ERR("VBO to VAO binding");

	// vertex position attribute
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(vertex),
		(void*)0
	);
	GL_ERR("vertex attrib");
	
	glEnableVertexAttribArray(0);
	GL_ERR("vertex attrib enable");

	// vertex uvs attribute
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(vertex),
		(void*)(3 * sizeof(float))
	);
	GL_ERR("uvs attrib");

	glEnableVertexAttribArray(1);
	GL_ERR("uvs attrib enable");

	// vertex normal attribute
	glVertexAttribPointer(
		2,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(vertex),
		(void*)(5 * sizeof(float))
	);
	GL_ERR("normal attrib");
	
	glEnableVertexAttribArray(2);
	GL_ERR("normal attrib enable");
}

// destroys the VBO and the VAO for this mesh
void destroyGLMeshes(mesh* mesh) {
	glDeleteBuffers(1, &mesh->vbo);
	glDeleteVertexArrays(1, &mesh->vao);
}

mesh* mesh_import(FILE* file) {
	// initializemesh 
	mesh* new_mesh = malloc(sizeof(mesh));
	memset(new_mesh, 0, sizeof(mesh));

	if(!meshDecode(new_mesh, file)) {
		free(new_mesh);
		return NULL;
	}

	// generate OpenGL buffers
	generateGLMeshes(new_mesh);

	return new_mesh;
}

void mesh_free(mesh* mesh) {
	if(!mesh) return;
	
	// free OpenGL buffers 
	destroyGLMeshes(mesh);	
	
	free(mesh->verts);
	free(mesh);
}

// mesh handler implementations
DATA_TABLE_IMPL(mesh)
