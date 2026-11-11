#include "helpers.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// -- utils

__attribute__((weak))
void* xmalloc(size_t size) {
	void* ptr = malloc(size);

	if(!ptr && size != 0) {
		printf("Memory allocation failed via malloc()\n");
		exit(1);
	}

	return ptr;
}

// macro for OpenGL errors
#define GL_ERR(func)               \
	{                              \
	    GLenum err = glGetError(); \
	    if(err != GL_NO_ERROR)     \
	    return 0;                  \
	}

// -- shaders

// GUI shader users 
int shaderRefs = 0;

// shader GLuint handle
GLuint shaderHandle;

// reads a shader file
char* guiReadShader(const char* path) {
	// open shader
	FILE* shaderFile = fopen(path, "r");
	if(shaderFile == NULL) return NULL;

	// read file size
	fseek(shaderFile, 0, SEEK_END);
	long size = ftell(shaderFile);
	rewind(shaderFile);

	// allocate buffer
	char *buf = xmalloc(size + 1);

	// store file in buffer and terminate
	fread(buf, 1, size, shaderFile);
	buf[size] = '\0';

	// close file
	fclose(shaderFile);

	return buf;
}

// macro for OpenGL shader compilation errors
#define GL_COMPILE_ERR(shader) \
	    glGetShaderiv(         \
	        shader,            \
	        GL_COMPILE_STATUS, \
	        &success           \
	    );                     \
	    if(!success) {         \
	        return 0;          \
	    }                      \

// macro for OpenGL shader linking errors
#define GL_LINK_ERR(program) \
	    glGetProgramiv(      \
	        program,         \
	        GL_LINK_STATUS,  \
	        &success         \
	    );                   \
	    if(!success) {       \
	        return 0;        \
	    }      

GLuint guiImportShader(const char* vertPath, const char* fragPath) {
	if(!shaderRefs) {
		// initialize buffers
		char* vertShader = guiReadShader(vertPath);		
		if(!vertShader) return 0;
		char* fragShader = guiReadShader(fragPath);
		if(!fragShader) return 0;

		// return on failure
		if(!vertShader || !fragShader) {
			free(vertShader);
			free(fragShader);
			return 0;
		}

		// compile shader
		GLint success = 1;

		// compile vertex shader
		GLuint vert = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(
			vert,
			1,
			(const char**)&vertShader,
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
			(const char**)&fragShader,
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

		// free buffers
		free(vertShader);
		free(fragShader);

		// setup handle
		shaderHandle = program;
	}

	shaderRefs++;
	return shaderHandle;
}

void guiFreeShader(GLuint program) {
	if(program != shaderHandle) return;
	if(shaderRefs == 0) return;

	shaderRefs--;

	if(shaderRefs == 0) {
		glDeleteProgram(
			program
		);
	}
}

// -- textures

// GUI texture users 
int textureRefs = 0;

// texture GLuint handle
GLuint textureHandle;

// gui .tga image header
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
} guiHeaderTGA;

GLuint guiImportTexture(const char* path) {
	if(!textureRefs) {
		FILE* textureFile = fopen(path, "r");
		if(textureFile == NULL) return 0;

		fseek(textureFile, 0, SEEK_SET);

		// read header
		guiHeaderTGA head;
		if (fread(&head, sizeof head, 1, textureFile) != 1) {
			fclose(textureFile);
			return 0;
		}

		// validate header (ignore orientation)
		if (head.imageType != 2
		|| head.colorMapType != 0
		|| (head.pixelDepth != 24 && head.pixelDepth != 32)
		|| head.width == 0
		|| head.height == 0
		|| (head.imageDescriptor & 0xC0) != 0) {
			fclose(textureFile);
			return 0;
		}

		// skip
		fseek(textureFile, head.idLength, SEEK_CUR);

		// get size to allocate
		size_t bpp = head.pixelDepth / 8;
		size_t pixels = head.width * head.height;
		size_t size = pixels * 4;
		uint8_t* data = xmalloc(size);

		// read texture
		for(size_t i = 0; i < pixels; i++) {
			uint8_t word[4];

			// read data
			if (fread(word, bpp, 1, textureFile) != 1) {
				fclose(textureFile);
				free(data);
				return 0;
			}

			// swizzle
			uint8_t b = word[0];
			uint8_t g = word[1];
			uint8_t r = word[2];
			uint8_t a = (bpp == 4) ? word[3] : 255;

			// throw data in texture
			size_t idx = i * 4;
			data[idx + 0] = r;
			data[idx + 1] = g;
			data[idx + 2] = b;
			data[idx + 3] = a;
		}
	
		// close file
		fclose(textureFile);

		// set width and height
		int width = head.width;
		int height = head.height;

		// generate and bind texture
		GLuint texture;
		glGenTextures(
			1,
			&texture
		);
		GL_ERR("texture generation");
		glBindTexture(
			GL_TEXTURE_2D,
			texture
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
			GL_LINEAR
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
			GL_SRGB8_ALPHA8,
			width,
			height,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			data
		);
		GL_ERR("texture assignment");

		// generate mip maps
		glGenerateMipmap(GL_TEXTURE_2D);
		GL_ERR("mip map generation");

		// free buffer
		free(data);

		// setup handle
		textureHandle = texture;
	}

	textureRefs++;
	return textureHandle;
}

void guiFreeTexture(GLuint texture) {
	if(texture != textureHandle) return;
	if(textureRefs == 0) return;

	textureRefs--;

	if(textureRefs == 0) {
		glDeleteTextures(
			1,
			&texture
		);
	}
}
