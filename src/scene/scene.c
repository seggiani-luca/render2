#include "scene.h"
#include "entity/entity.h"
#include "../render/render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -- scenes

scene* newScene(const char* name) {
	if(*name == '\0') return NULL;

	// allocate scene
	scene* s = malloc(sizeof(scene));
	if(!s) return NULL;

	// copy name and clear root
	strncpy(s->name, name, ENT_NAME_SIZ);
	s->name[ENT_NAME_SIZ - 1] = '\0';

	// setup root
	strcpy(s->root.name, ROOT_NAME);
	s->root.root = NULL;
	s->root.fieldCount = 0;
	s->root.parent = s->root.child = s->root.peer = NULL;
	s->root.childCount = 0;

	// clear render scene
	s->render = (renderScene){0};
	s->dirty = 1;

	return s;
}

void initDefaultScene(scene* s) {
	if(!s) return;

	// default camera
	appendChild(&s->root, newCameraEntity("Camera"));
	
	// default camera
	appendChild(&s->root, newSunEntity("Sun"));
	
	// default entity
	entity* ent = newRenderableEntity("Entity");

	// set default mesh
	meshField* mesh = (meshField*)getField(ent, REN_MESH_NAME);
	mesh->ref = meshImport(DEF_MESH);

	// set default material
	materialField* material = (materialField*)getField(ent, REN_MATERIAL_NAME);
	material->ref = materialImport(DEF_MATERIAL);

	appendChild(&s->root, ent);
}

void freeScene(scene* s) {
	freeRenderScene(s);
	freeEntityChildren(&s->root);
	free(s);
}

sceneIter getScIter(scene* s) {
	return (sceneIter){
		&s->root, 0
	};
}

entity* scIterNext(sceneIter* it) {
	if(!it->cur) return NULL;

	// first check children
	if(it->cur->child) {
		it->depth++;
		it->cur = it->cur->child;
		return it->cur;
	}

	// climb to first parent with peers
	while(it->cur && !it->cur->peer) {
		it->depth--;
		it->cur = it->cur->parent;
	}

	// return NULL when root reached
	if(!it->cur) return NULL;

	// go to peer
	it->cur = it->cur->peer;
	return it->cur;
}
