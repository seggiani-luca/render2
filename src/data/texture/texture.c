#include "texture.h"
#include "../../render/render.h"
#include <stdlib.h>
#include <string.h>

extern int textureDecode(texture* texture, FILE* file, int* rgba);

// -- textures

void textureFilter(texture* tex, int linear) {
	glBindTexture(GL_TEXTURE_2D, tex->tex);

	// set the texture filtering options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER,
		linear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST
	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER,
		linear ? GL_LINEAR : GL_NEAREST
	);
}

void texturePrint(void* dat) {
	texture* tex = (texture*)dat;
	printf("Texture (width: %d, height: %d)", tex->width, tex->height);
}

// generates an OpenGL texture for a texture
void generateGLTextures(texture* texture, int rgba) {
	// generate and bind texture
	glGenTextures(
		1,
		&texture->tex
	);
	GL_ERR("texture generation");
	glBindTexture(
		GL_TEXTURE_2D,
		texture->tex
	);
	GL_ERR("texture generation binding");

	// set the texture wrapping/filtering options
	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_S,
		GL_REPEAT
	);
	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_T,
		GL_REPEAT
	);
	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR
	);
	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER,
		GL_LINEAR	
	);
	GL_ERR("texture parameters")

	// assign texture
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		rgba ? GL_RGBA : GL_RGB,
		texture->width,
		texture->height,
		0,
		rgba ? GL_RGBA : GL_RGB,
		GL_UNSIGNED_BYTE,
		texture->data
	);
	GL_ERR("texture assignment");

	// generate mip maps
	glGenerateMipmap(GL_TEXTURE_2D);
	GL_ERR("mip map generation");
}

// destroys the OpenGL texture for this texture
void destroyGLTextures(texture* texture) {
	glDeleteTextures(
		1,
		&texture->tex
	);
}

void* texture_import(FILE* file) {
	// initialize texture
	texture* new_texture = malloc(sizeof(texture));
	memset(new_texture, 0, sizeof(texture));

	// load texture
	int rgba;
	if(!textureDecode(new_texture, file, &rgba)) {
		free(new_texture);
		return NULL;
	}

	// generate OpenGL texture
	generateGLTextures(new_texture, rgba);

	return new_texture;
}

void texture_free(void* dat) {
	if(!dat) return;

	// free OpenGL texture
	destroyGLTextures(dat);

	free(((texture*)dat)->data);
	free(dat);
}

// texture handler implementations
DATA_TABLE_IMPL(texture)
