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
} headerTGA;

// imports a texture in .tga format
int textureDecode(texture *texture, FILE *file) {
  fseek(file, 0, SEEK_SET);

  // read header
  headerTGA head;
  fread(&head, sizeof(headerTGA), 1, file);

  // validate header
  if (head.imageType != 2 || (head.pixelDepth != 32 && head.pixelDepth != 24) ||
      ((head.imageDescriptor & 0x20) != 0) || head.colorMapType != 0)
    return 0;

  // skip
  fseek(file, head.idLength, SEEK_CUR);

  // get size to allocate
  int bpp = head.pixelDepth / 8;
  int size = head.width * head.height * bpp;
  texture->data = malloc(size);
  if (!texture->data)
    return 0;

  // read texture
  for (int i = 0; i < size / bpp; i++) {
    uint8_t data[4];
    fread(data, bpp, 1, file);

    uint8_t b = data[0];
    uint8_t g = data[1];
    uint8_t r = data[2];
    uint8_t a = (bpp == 4) ? data[3] : 255;

    int idx = i * bpp;
    texture->data[idx + 0] = r;
    texture->data[idx + 1] = g;
    texture->data[idx + 2] = b;
    if (bpp == 4)
      texture->data[idx + 3] = a;
  }

  texture->width = head.width;
  texture->height = head.height;

  return 1;
}
