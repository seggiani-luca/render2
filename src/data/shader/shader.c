#include "shader.h"
#include "../../render/render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// reads a shader file
char* readShader(const char* path) {
	// open shader
	FILE* shaderFile = fopen(path, "r");
	if(shaderFile == NULL) return NULL;

	// read file to end
	fseek(shaderFile, 0, SEEK_END);
	long size = ftell(shaderFile);
	fseek(shaderFile, 0, SEEK_SET);

	// allocate buffer
	char* shader = malloc(size + 1);

	// read file from beginning
	fread(shader, 1, size, shaderFile);
	shader[size] = '\0';

	// close file
	fclose(shaderFile);

	return shader;
}

// compiles a vert and a frag shader into a program and returns it
GLuint compileShaders(const char* vertShader, const char* fragShader) {
	// compile vertex shader
	GLuint vert = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(
		vert,
		1,
		&vertShader,
		NULL
	);
	GL_ERR("vert source");

	glCompileShader(vert);
	GL_COMPILE_ERR(vert);
	GL_ERR("vert compilation");

	// compile fragment shader
	GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(
		frag,
		1,
		&fragShader,
		NULL
	);
	GL_ERR("frag source");

	glCompileShader(frag);
	GL_COMPILE_ERR(frag);
	GL_ERR("frag compilation");

	// create program
	GLuint program = glCreateProgram();

	// attach vertex shader
	glAttachShader(
		program,
		vert
	);
	GL_ERR("vert attach");

	// attach fragment shader
	glAttachShader(
		program,
		frag
	);
	GL_ERR("frag attach");

	// link program
	glLinkProgram(program);
	GL_LINK_ERR(program);
	GL_ERR("program linking");

	// delete shaders
	glDeleteShader(vert);
	glDeleteShader(frag);

	return program;
}

void shaderPrint(void* dat) {
	shader* shd = (shader*)dat;
	printf("Shader %d\n", shd->program);
}

void shader_free(shader* shader) {
	if(shader == NULL) return;

	// free OpenGL program
	glDeleteProgram(shader->program);

	free(shader);
}

// shader handler implementations
dataTable shaderTable = {
	.root = NULL,
	.print = (void (*)(void*))shaderPrint,
	.import = NULL,
	.free = (void (*)(void*))shader_free
};

// custom shader importer
dataRef* shaderImport(const char* vert, const char* frag) {
	// make composite path
	char path[DAT_PATH_SIZ];
	snprintf(path, DAT_PATH_SIZ, "%s,%s", vert, frag);

	// query table by paths
	dataRef* ref = shaderTable.root;
	while(ref) {
		// return if found
		if(strcmp(ref->path, path) == 0) {
			ref->refCount++;
			return ref;
		}

		ref = ref->next;
	}

	// initialize buffers
	char* vert_shader = readShader(vert);
	char* frag_shader = readShader(frag);

	// return NULL on failure
	if(!vert_shader || !frag_shader) {
		free(vert_shader);
		free(frag_shader);
		return NULL;
	}

	// compile shader
	GLuint program = compileShaders(
		vert_shader,
		frag_shader
	);

	// free buffers
	free(vert_shader);
	free(frag_shader);

	// create new shader
	shader* new_shader = malloc(sizeof(shader));
	memset(new_shader, 0, sizeof(shader));
	new_shader->program = program;

	// walk table to end
	dataRef** cur = &shaderTable.root;
	while(*cur) cur = &(*cur)->next;
	
	// allocate entry
	dataRef* newRef = malloc(sizeof(dataRef));
	if(!newRef) return NULL;

	// insert in table
	newRef->next = NULL;
	*cur = newRef;

	// copy data
	strncpy(newRef->path, path, DAT_PATH_SIZ);
	newRef->path[DAT_PATH_SIZ - 1] = '\0';
	newRef->refCount = 1;
	newRef->data = new_shader;

	// return data
	return newRef;
}

void shaderFree(void* dat) {
	freeData(dat, &shaderTable);
}
