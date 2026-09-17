#include "texture.h"
#include "../../exception/exception.h"
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
int textureDecode(texture* tex, FILE* file) {
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
		logEvent(ERROR, IO, ".tga header invalid");
		return 0;
	}

	// skip
	fseek(file, head.idLength, SEEK_CUR);

	// get size to allocate
	size_t bpp = head.pixelDepth / 8;
	size_t pixels = head.width * head.height;
	size_t size = pixels * 4;
	tex->data = xmalloc(size);

	// read texture
	for(size_t i = 0; i < pixels; i++) {
		uint8_t data[4];

		// read data
		if (fread(data, bpp, 1, file) != 1) {
			free(tex->data);
			tex->data = NULL;
			logEvent(ERROR, IO, ".tga data section too short");
			return 0;
		}

		// swizzle
		uint8_t b = data[0];
		uint8_t g = data[1];
		uint8_t r = data[2];
		uint8_t a = (bpp == 4) ? data[3] : 255;

		// throw data in texture
		size_t idx = i * 4;
		tex->data[idx + 0] = r;
		tex->data[idx + 1] = g;
		tex->data[idx + 2] = b;
		tex->data[idx + 3] = a;
	}

	// set width and height
	tex->width = head.width;
	tex->height = head.height;

	return 1;
}
