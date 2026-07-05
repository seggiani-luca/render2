#ifndef DATA_TEXTURE_H
#define DATA_TEXTURE_H

#include "../../../lib/glad/glad.h"
#include "../data.h"
#include <stdint.h>

/* ---- texture types ------------------------------------------------------ */

// texture data type
typedef struct {
	// texture dimensions
	int width, height;

	// texture data
	uint8_t* data;

	// OpenGL texture handle
	GLuint tex; 
} texture;

// print texture info
void texturePrint(texture* texture);

// texture handler declaration
DATA_TABLE_DECL(texture)

#endif
