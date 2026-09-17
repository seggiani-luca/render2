#include "material.h"
#include "../decode.h"

// size of file line
#define MATERIAL_LINE_SIZ 256

// parses texture from .mtl file
int parseTex(char* line, const char* key, texture** tex, int srgb) {
	char path[DAT_PATH_SIZ];

	// only if key found
	if(parseStringKey(line, key, DAT_PATH_SIZ, path)) {
		// import texture
		dataRef* ref = textureImport(path);
		if(!ref) {
			logEvent(ERROR, IO, "Couldn't load material texture at %s", path);
			return 0;
		} 

		// set to SRGB if needed
		if(srgb) textureColor(ref->data, 1);

		// assign
		*tex = ref->data;

		return 1;
	}

	return 1; // not present, not an error
}

// imports a material in .mtl format
int materialDecode(material* mat, FILE* file) {
	char line[MATERIAL_LINE_SIZ];

	// buffers for vert and frag paths
	char vertPath[DAT_PATH_SIZ] = {0};
	char fragPath[DAT_PATH_SIZ] = {0};

	while(fgets(line, MATERIAL_LINE_SIZ, file)) {
		// get shaders
		parseStringKey(line, "vert", DAT_PATH_SIZ, vertPath);
		parseStringKey(line, "frag", DAT_PATH_SIZ, fragPath);

		// get colors
		parseFloatKey(line, "Kd", 3, (float*)&mat->diffuseCol );
		parseFloatKey(line, "Ks", 3, (float*)&mat->specularCol);
		parseFloatKey(line, "Kb", 3, (float*)&mat->subsurfCol );
		parseFloatKey(line, "Ns", 1,         &mat->shininess  );

		// get maps
		if(!parseTex(line, "map_Kd", &mat->diffuseMap,   1)) return 0;
		if(!parseTex(line, "map_Ks", &mat->specularMap,  0)) return 0;
		if(!parseTex(line, "map_Ns", &mat->shininessMap, 0)) return 0;
	}

	// get matching shader
	dataRef* ref = shaderImport(vertPath, fragPath);
	if(ref) mat->shader = ref->data;
	else {
		logEvent(ERROR, IO,
			"Couldn't load material shader at paths vert: %s, frag: %s",
			vertPath,
			fragPath
		);
		return 0;
	} 

	return 1;
}
