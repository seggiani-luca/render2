#include "render.h"
#include "../scene/scene.h"
#include <stdlib.h>
#include <string.h>

// -- rendering

// context for scene rendering 
typedef struct {
	// scene
	scene* scn;
} renderingContext;

// actually renders a scene
void render(window* win) {
	// get context
	renderingContext* rCtx = (renderingContext*) win->cbak.ctx;
	scene* scn = rCtx->scn;
	renderScene* rnd = &scn->render;

	// get scene data
	camera* camInfo = rnd->camera.info;
	mat4 camTrans = rnd->camera.transform;
	atmosphere* atmos = rnd->atmosphere;

	// only if valid
	if(!camInfo) return;
	if(!atmos) return;

	// clear buffer
	glClearColor(
		atmos->background.r,
		atmos->background.g,
		atmos->background.b,
		1.0
	);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// get view and projection matrices
	mat4 view = matInv4(camTrans);
	mat4 proj = matPersp(
		camInfo->fov,
		camInfo->nearPlane,
		camInfo->farPlane,
		win->fbWidth / win->fbHeight
	);

	// go through all render entities
	renderEntity* cur = rnd->root;
	while(cur) {
		// render entity

		cur = cur->next;
	}
}

renderCallback makeRenderCallback(scene* scn) {
	// initialize context
	renderingContext* rCtx = malloc(sizeof(renderingContext));
	rCtx->scn = scn;

	// return callback
	return (renderCallback){
		render,
		rCtx,
		free
	};
}

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

		// reset stack
		.stack.vec[0] = transformToMat4(transformIdent()),

		// reset stack
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
	if (it->stack.cur < 0 || it->stack.cur >= SCN_MAX_DEPTH) return NULL;

	// get transform
	field* f = getField(new, REN_TRANSFORM_NAME);
	transform tf;
	if(f) tf = ((transformField*)f)->val;
	else tf = transformIdent();

	// convert to matrix 
	mat4 local = transformToMat4(tf);

	// compose with previous transform
	it->stack.vec[it->stack.cur] =
		matMul4(it->stack.vec[it->stack.cur - 1], local);

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

	// clear render scene
	memset(scn, 0, sizeof(renderScene));
}

void updateRenderScene(scene* scene) {
	renderScene* render = &scene->render;

	// clear scene
	freeRenderScene(render);
	scene->dirty = 0;

	// go through scene hierarchy
	renderSceneIter iter = getRenderScIter(scene);
	do {
		entity* cur = iter.base.cur;
		mat4 curTrans = renderScTransform(&iter);

		// get transform (field)
		field* transform = getField(cur, REN_TRANSFORM_NAME);

		// get mesh
		field* meshF = getField(cur, REN_MESH_NAME);
		mesh* mesh = NULL;
		if(meshF) if(((meshField*)meshF)->ref)
			mesh = ((meshField*)meshF)->ref->data;

		// get material 
		field* matF = getField(cur, REN_MATERIAL_NAME);
		material* material = NULL;
		if(matF) if(((materialField*)matF)->ref)
			material = ((materialField*)matF)->ref->data;
		
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
	} while(renderScIterNext(&iter));
}

void printRenderScene(scene* scene) {
	renderScene* render = &scene->render;

	printf("%s (Render View):\n", scene->name);

	// print camera
	if(render->camera.info) {
		printf("Camera Info: Fov: %g, Near: %g, Far: %g\n",
			render->camera.info->fov,
			render->camera.info->nearPlane,
			render->camera.info->farPlane);

		printf("Camera Transform:\n");
		matPrint4(render->camera.transform); printf("\n");
	}

	// atmosphere
	if(render->atmosphere) {
		printf("Atmosphere: Ambient: %f, %f, %f\n",
			render->atmosphere->ambient.r,
			render->atmosphere->ambient.g,
			render->atmosphere->ambient.b);
	}

	// render entities
	printf("Render Entities:\n");
	renderEntity* cur = render->root;
	while(cur) {
		printf("Mesh: %p\n",     cur->mesh    );
		printf("Material: %p\n", cur->material);

		printf("Transform:\n");
		matPrint4(cur->transform);

		printf("\n");
		cur = cur->next;
	}
		
	printf("\n");
}
