#ifndef DATA_TEXTURE_H
#define DATA_TEXTURE_H

#include "../../../lib/glad/glad.h"
#include "../data.h"
#include <stdint.h>

// -- textures

// texture data type
typedef struct {
	// texture dimensions
	int width, height;

	// texture data
	uint8_t* data;

	// OpenGL texture handle
	GLuint tex;
} texture;

// set the filtering options for a texture
void textureFilter(texture* tex, int linear);

// print texture info
void texturePrint(void* dat);

// texture handler declarations
DATA_TABLE_DECL(texture)

#endif
