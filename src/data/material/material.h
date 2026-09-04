#ifndef DATA_MATERIAL_H
#define DATA_MATERIAL_H

#include "../data.h"
#include "../shader/shader.h"
#include "../texture/texture.h"
#include "../../math/math.h"
#include "../../../lib/glad/glad.h"

// -- material 

// material data type
typedef struct {
	// material shader
	shader* shader;

	// diffuse color
	color diffuseCol;

	// diffuse map 
	texture* diffuseMap;

	// specular color
	color specularCol;

	// specular map 
	texture* specularMap;

	// specular exponent
	float shininess;
	
	// specular exponent map 
	texture* shininessMap;
	
	// subsurface color
	color subsurfCol;
} material;

// print material info
void materialPrint(void* dat);

// material handler declarations
DATA_TABLE_DECL(material)

#endif
