#include "material.h"
#include "../decode.h"
#include "../../render/render.h"

// size of file line
#define MATERIAL_LINE_SIZ 256

// parses texture from .mtl file
int parseTex(char* line, const char* key, texture** tex) {
	char path[DAT_PATH_SIZ];

	// only if key found
	if(parseStringKey(line, key, path)) {
		// import texture
		dataRef* ref = textureImport(path);
		if(!ref) return 0;

		// assign
		*tex = ref->data;

		return 1;
	}

	return 1;
}

// imports a material in .mtl format
int materialDecode(material* material, FILE* file) {
	char line[MATERIAL_LINE_SIZ];

	// buffers for vert and frag paths
	char vertPath[DAT_PATH_SIZ];
	char fragPath[DAT_PATH_SIZ];

	while(fgets(line, MATERIAL_LINE_SIZ, file)) {
		// get shaders
		parseStringKey(line, "vert", vertPath);
		parseStringKey(line, "frag", fragPath);

		// get colors
		parseFloatKey(line, "Kd ", 3, (float*)&material->diffuseCol);
		parseFloatKey(line, "Ks ", 3, (float*)&material->specularCol);
		parseFloatKey(line, "Kb ", 3, (float*)&material->subsurfCol);
		parseFloatKey(line, "Ns ", 1, &material->shininess);

		// get maps
		if(!parseTex(line, "map_Kd", &material->diffuseMap)) return 0;
		if(!parseTex(line, "map_Ks", &material->specularMap)) return 0;
		if(!parseTex(line, "map_Ns", &material->shininessMap)) return 0;
	}

	// get matching shader
	dataRef* ref = shaderImport(vertPath, fragPath);
	if(ref) material->shader = ref->data;
	else return 0;

	return 1;
}
