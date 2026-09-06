#include "texture.h"
#include <stdlib.h>

// .tga image header
typedef struct __attribute__((packed)) {
	uint8_t idLength;
	uint8_t colorMapType;
	uint8_t imageType;

	uint16_t colorMapStart;
	uint16_t colorMapLength;
	uint8_t colorMapDepth;

	uint16_t xOrigin;
	uint16_t yOrigin;

	uint16_t width;
	uint16_t height;

	uint8_t pixelDepth;
	uint8_t imageDescriptor;
}
headerTGA;

// imports a texture in .tga format
int textureDecode(texture* texture, FILE* file) {
	fseek(file, 0, SEEK_SET);

	// read header
	headerTGA head;
	if (fread(&head, sizeof head, 1, file) != 1) return 0;

	// validate header (ignore orientation)
	if (head.imageType != 2
    || head.colorMapType != 0
    || (head.pixelDepth != 24 && head.pixelDepth != 32)
    || head.width == 0
    || head.height == 0
    || (head.imageDescriptor & 0xC0) != 0) {
		return 0;
	}

	// skip
	fseek(file, head.idLength, SEEK_CUR);

	// get size to allocate
	size_t bpp = head.pixelDepth / 8;
	size_t pixels = head.width * head.height;
	size_t size = pixels * 4;
	texture->data = malloc(size);
	if(!texture->data) return 0;

	// read texture
	for(size_t i = 0; i < pixels; i++) {
		uint8_t data[4];

		// read data
		if (fread(data, bpp, 1, file) != 1) {
			free(texture->data);
			texture->data = NULL;
			return 0;
		}

		// swizzle
		uint8_t b = data[0];
		uint8_t g = data[1];
		uint8_t r = data[2];
		uint8_t a = (bpp == 4) ? data[3] : 255;

		// throw data in texture
		int idx = i * 4;
		texture->data[idx + 0] = r;
		texture->data[idx + 1] = g;
		texture->data[idx + 2] = b;
		texture->data[idx + 3] = a;
	}

	// set width and height
	texture->width = head.width;
	texture->height = head.height;

	return 1;
}
