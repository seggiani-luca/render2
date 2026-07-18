#include "material.h"
#include <stdlib.h>
#include <string.h>

extern int materialDecode(material* material, FILE* file);

void materialPrint(void* dat) {
	material* mat = (material*)dat;
	printf("Material (shininess: %f)", mat->shininess);
}

void* material_import(FILE* file) {
	// initialize material 
	material* new_material = malloc(sizeof(material));
	memset(new_material, 0, sizeof(material));

	if(!materialDecode(new_material, file)) {
		free(new_material);
		return NULL;
	}

	return new_material;
}

void material_free(void* dat) {
	if(!dat) return;

	material* mat = (material*)dat;
	if(mat->diffuseMap) textureFree(mat->diffuseMap);
	if(mat->specularMap) textureFree(mat->specularMap);

	free(dat);
}

// material handler implementations
DATA_TABLE_IMPL(material)
