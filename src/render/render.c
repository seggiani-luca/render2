#include "render.h"
#include "../scene/scene.h"
#include <stdlib.h>
#include <string.h>

// -- rendering

// context for scene rendering 
typedef struct {
	// scene
	scene* scn;

	// substruct for OpenGL specific context
	struct {
		// VAO for sky cube 
		GLuint skyVAO;

		// VBO for basic cube 
		GLuint skyVBO;
		
		// skybox shader 
		shader* shd;
	} gl;
} renderingContext;

// initializes render OpenGL data
int newRender(renderingContext* rCtx) {
	float cubeTris[] = {
		// back
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		// front
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		// left
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,

		// right
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,

		// bottom
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,

		// top
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f
	};

	// initialize basic cube VAO
	glGenVertexArrays(
		1,
		&rCtx->gl.skyVAO
	);
	GL_ERR("skybox VAO generation");
	glBindVertexArray(rCtx->gl.skyVAO);
	GL_ERR("skybox VAO binding");

	// initialize basic cube VBO
	glGenBuffers(
		1,
		&rCtx->gl.skyVBO
	);
	GL_ERR("skybox VBO generation");
	glBindBuffer(
		GL_ARRAY_BUFFER,
		rCtx->gl.skyVBO
	);
	GL_ERR("skybox VBO binding");

	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(cubeTris),
		cubeTris,
		GL_STATIC_DRAW
	);
	GL_ERR("skybox VBO data");

	// cube vertex attribute
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		3 * sizeof(float),
		(void*)0
	);
	GL_ERR("skybox vertex attrib");
	glEnableVertexAttribArray(0);
	GL_ERR("skybox vertex attrib enable");
	
	// import shader
	rCtx->gl.shd = shaderImport(SKY_VERT_PATH, SKY_FRAG_PATH)->data;
	if(!rCtx->gl.shd) return 0;

	return 1;
}

// renders the skybox
void doRenderSkybox(
	renderingContext* ctx,
	atmosphere* atmInfo,
	mat4 view,
	mat4 proj
) {
	// get shader and material
	shader* shader = ctx->gl.shd;

	// extract translation from view matrix
	view.d = 0.0;
	view.h = 0.0;
	view.l = 0.0;

	// setup program
	glUseProgram(shader->program);
	GL_ERR("sky program selection");

	// reset texture unit allocator for this draw
	resetTexUnit();

	// send transform matrices
	sendUniform(shader, VIEW,          &view.a);
	sendUniform(shader, PROJECTION,    &proj.a);

	// send cubemap
	sendUniform(shader, DIFFUSE_COLOR, &atmInfo->background.r              );
	sendUniform(shader, DIFFUSE_MAP,    atmInfo->backgroundMap ? 
	                                    atmInfo->backgroundMap->data : NULL);


	// setup VAO
	glBindVertexArray(ctx->gl.skyVAO);
	GL_ERR("sky draw call VAO binding");

	// issue draw call
	glDisable(GL_CULL_FACE);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glEnable(GL_CULL_FACE);
	GL_ERR("sky draw call issue");
}

// actually renders an entity
void doRenderEntity(
	renderEntity* ent,
	camera* camInfo __attribute__((unused)),
	mat4 camTrans,
	atmosphere* atmInfo,
	mat4 atmTrans,
	mat4 view,
	mat4 proj
) {
	// get model matrix
	mat4 model = ent->transform;

	// get shader and material
	material* material = ent->material;
	shader* shader = material->shader;

	// setup program
	glUseProgram(shader->program);
	GL_ERR("program selection");

	// reset texture unit allocator for this draw
	resetTexUnit();

	// send transform matrices
	sendUniform(shader, MODEL,             &model.a);
	sendUniform(shader, VIEW,              &view.a );
	sendUniform(shader, PROJECTION,        &proj.a );

	// send camera
	sendUniform(shader, CAMERA_POSITION,     mat4ExPosition(&camTrans));

	// send sun
	sendUniform(shader, SUN_DIRECTION,      mat4ExForward(&atmTrans));
	sendUniform(shader, SUN_COLOR,         &atmInfo->sun.r          );

	// send ambient
	sendUniform(shader, AMBIENT_COLOR,     &atmInfo->ambient.r       );
	sendUniform(shader, AMBIENT_MAP,        atmInfo->ambientMap ? 
	                                        atmInfo->ambientMap->data : NULL);

	// send diffuse
	sendUniform(shader, DIFFUSE_COLOR,     &material->diffuseCol.r);
	sendUniform(shader, DIFFUSE_MAP,        material->diffuseMap  );

	// send specular
	sendUniform(shader, SPECULAR_COLOR,    &material->specularCol.r);
	sendUniform(shader, SPECULAR_MAP,       material->specularMap  );

	// send shininess
	sendUniform(shader, SHININESS,         &material->shininess    );
	sendUniform(shader, SHININESS_MAP,      material->shininessMap );

	// send subsurface
	sendUniform(shader, SUBSURFACE_COLOR,  &material->subsurfCol.r);

	// setup VAO
	glBindVertexArray(ent->mesh->vao);
	GL_ERR("draw call VAO binding");

	// issue draw call
	glDrawArrays(GL_TRIANGLES, 0, ent->mesh->vertCount);
	GL_ERR("draw call issue");
}

// actually renders a scene
void render(window* win) {
	// get context
	renderingContext* rCtx = (renderingContext*) win->cbak.ctx;
	scene* scn = rCtx->scn;
	renderScene* rnd = &scn->render;

	// create OpenGL objects if needed
	if(!rCtx->gl.shd) newRender(rCtx);

	// update render scene if modified
	if(scn->dirty) updateRenderScene(scn);

	// get scene data
	camera* camInfo = rnd->camera.info;
	mat4 camTrans = rnd->camera.transform;
	atmosphere* atmInfo = rnd->atmosphere.info;
	mat4 atmTrans = rnd->atmosphere.transform;

	// only if valid
	if(!camInfo) return;
	if(!atmInfo) return;

	// clear buffer
	glClearColor(
		atmInfo->background.r,
		atmInfo->background.g,
		atmInfo->background.b,
		1.0
	);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// get view and projection matrices
	mat4 view = matInv4(camTrans);
	mat4 proj = matPersp(
		camInfo->fov,
		camInfo->nearPlane,
		camInfo->farPlane,
		(float)win->fbWidth / win->fbHeight
	);

	// set entity depth
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	// go through all render entities
	renderEntity* cur = rnd->root;
	while(cur) {
		// render entity
		doRenderEntity(
			cur,
			camInfo,
			camTrans,
			atmInfo,
			atmTrans,
			view,
			proj
		);

		cur = cur->next;
	}

	// set skybox depth
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);

	// render skybox
	doRenderSkybox(rCtx, atmInfo, view, proj);

	// set entity depth again
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
}

// frees a render context
void freeRender(void* vCtx) {
	renderingContext* rCtx = (renderingContext*)vCtx;

	// free VBOs and VAO
	glDeleteBuffers(
		1,
		&rCtx->gl.skyVBO
	);
	glDeleteVertexArrays(
		1,
		&rCtx->gl.skyVAO
	);

	// free shader
	shaderFree(rCtx->gl.shd);

	free(rCtx);
}

renderCallback makeRenderCallback(scene* scn) {
	// initialize context
	renderingContext* rCtx = malloc(sizeof(renderingContext));
	rCtx->scn = scn;
	rCtx->gl.shd = NULL; // flag via shader

	// return callback
	return (renderCallback){
		render,
		rCtx,
		freeRender
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

void freeRenderScene(scene* scene) {
	renderScene* rnd = &scene->render;

	// go through all entities, freeing
	renderEntity* cur = rnd->root;
	while(cur) {
		renderEntity* tmp = cur;
		cur = cur->next;
		free(tmp);
	}

	// clear render scene
	memset(rnd, 0, sizeof(renderScene));
}

void updateRenderScene(scene* scene) {
	renderScene* render = &scene->render;

	// clear scene
	freeRenderScene(scene);
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
			render->atmosphere.info = atmosphere;
			render->atmosphere.transform = curTrans;
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
	if(render->atmosphere.info) {
		printf("Atmosphere Info: Ambient: %f, %f, %f\n",
			render->atmosphere.info->ambient.r,
			render->atmosphere.info->ambient.g,
			render->atmosphere.info->ambient.b);
		
		printf("Atmosphere Transform:\n");
		matPrint4(render->atmosphere.transform); printf("\n");
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
