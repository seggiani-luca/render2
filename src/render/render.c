#include "render.h"
#include "../scene/scene.h"
#include <stdlib.h>

// -- views

// max depth of scenes
#define SCN_MAX_DEPTH 16

// rendering scene iterator
typedef struct {
	sceneIter base;

	// last depth 
	int lastDepth;

	// stack of transforms
	struct {
		// stack vector
		mat4 vec[SCN_MAX_DEPTH];

		// stack current pointer
		int cur;
	} stack;
} renderSceneIter;

// gets a rendering scene iterator for a scene
renderSceneIter getRenderScIter(scene* s) {
	sceneIter base = getScIter(s);

	return (renderSceneIter){
		.base = base,
		.lastDepth = 0,
		.stack.cur = 0	
	};
}

// advances a render scene iterator
entity* renderScIterNext(renderSceneIter* it) {
	entity* new = scIterNext(&it->base);
	if(!new) return NULL;

	// compute delta
	int depthDelta = it->base.depth - it->lastDepth;

	// update stack cursor
	it->stack.cur += depthDelta;

	// get transform
	field* f = getField(new, REN_TRANSFORM_NAME);
	transform tf;
	if(f) tf = ((transformField*)f)->val;
	else tf = transformIdent();

	// compose transform
	mat4 local = transformToMat4(tf);
	if(it->stack.cur > 0) {
		// compose
		it->stack.vec[it->stack.cur] =
			matMul4(it->stack.vec[it->stack.cur - 1], local);
	} else {
		// start anew
		it->stack.vec[0] = local;
	}

	it->lastDepth = it->base.depth;
	return new;
}

// gets current transform from render scene iterator
mat4 renderScTransform(renderSceneIter* it) {
	return it->stack.vec[it->stack.cur];
}

// appens render entity to render scene
void appendRenderEntity(renderScene* scn, renderEntity* ent) {
	// get to last entity
	renderEntity** cur = &scn->root;
	while(*cur) cur = &(*cur)->next;

	// append
	ent->next = NULL;
	*cur = ent;
}

// frees render scene hierarchy
void freeRenderScene(renderScene* scn) {
	// go through all entities, freeing
	renderEntity* cur = scn->root;
	while(cur) {
		renderEntity* tmp = cur;
		cur = cur->next;
		free(tmp);
	}

	// clear root
	scn->root = NULL;
}

void updateRenderScene(scene* scene) {
	renderScene* render = &scene->render;

	// clear scene
	freeRenderScene(render);

	// go through scene hierarchy
	renderSceneIter iter = getRenderScIter(scene);
	for(;;) {
		entity* cur = iter.base.cur;
		mat4 curTrans = renderScTransform(&iter);

		// get transform (field)
		field* transform = getField(cur, REN_TRANSFORM_NAME);

		// get mesh
		field* meshF = getField(cur, REN_MESH_NAME);
		mesh* mesh = NULL;
		if(meshF) mesh = ((meshField*)meshF)->ref->data;

		// get material 
		field* matF = getField(cur, REN_MATERIAL_NAME);
		material* material = NULL;
		if(matF) material = ((materialField*)matF)->ref->data;
		
		// get camera 
		field* cameraF = getField(cur, REN_CAMERA_NAME);
		camera* camera = NULL;
		if(cameraF) camera = &((cameraField*)cameraF)->val;
		
		// get atmosphere 
		field* atmosF = getField(cur, REN_ATMOSPHERE_NAME);
		atmosphere* atmosphere = NULL;
		if(atmosF) atmosphere = &((atmosphereField*)atmosF)->val;

		// get if camera
		if(transform && camera) {
			render->camera.info = camera;
			render->camera.transform = curTrans;
		}

		// get if atmosphere
		if(atmosphere) {
			render->atmosphere = atmosphere;
		}

		// get if object
		if(transform && mesh && material) {
			// create entity
			renderEntity* ent = malloc(sizeof(renderEntity));
			ent->transform = curTrans;
			ent->mesh = mesh;
			ent->material = material;

			// append
			appendRenderEntity(render, ent);
		}

		// advance
		renderScIterNext(&iter);
	}
}
