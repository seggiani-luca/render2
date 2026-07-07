#include "texture.h"
#include "../../render/render.h"
#include <stdlib.h>
#include <string.h>

extern int textureDecode(texture* texture, FILE* file);

void texturePrint(texture* texture) {
	printf("Texture (width: %d, height: %d)", texture->width, texture->height);
}

// generates an OpenGL texture for a texture
void generateGLTextures(texture* texture) {
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
		GL_NEAREST_MIPMAP_NEAREST
	);
	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER,
		GL_NEAREST
	);
	GL_ERR("texture parameters")

	// assign texture
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		texture->width,
		texture->height,
		0,
		GL_RGBA,
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

texture* texture_import(FILE* file) {
	// initialize texture
	texture* new_texture = malloc(sizeof(texture));
	memset(new_texture, 0,sizeof(texture));

	if(!textureDecode(new_texture, file)) {
		free(new_texture);
		return NULL;
	}

	// generate OpenGL texture
	generateGLTextures(new_texture);

	return new_texture;
}

void texture_free(texture* texture) {
	if(!texture) return;

	// free OpenGL texture
	destroyGLTextures(texture);

	free(texture->data);
	free(texture);
}

// texture handler implementations
DATA_TABLE_IMPL(texture)
