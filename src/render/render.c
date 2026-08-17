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

// debug print a 4x4 uniform matrix
void debugUniformMatrix(GLuint program, GLint location, const char *name) {
    GLfloat m[16];

	// get uniform
    glGetUniformfv(program, location, m);

	// print matrix
    printf("%s:\n", name);
    for (int row = 0; row < 4; row++) {
        printf("  ");
        for (int col = 0; col < 4; col++)
            printf("%g ", m[col * 4 + row]);
        printf("\n");
    }
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

	// send model matrix
	glUniformMatrix4fv(
		shader->uniformLocations[MODEL], 
		1, GL_FALSE, &model.a);
	GL_ERR("uModel uniform");

	// send view matrix
	glUniformMatrix4fv(
		shader->uniformLocations[VIEW], 
		1, GL_FALSE, &view.a);
	GL_ERR("uView uniform");
	
	// send projection matrix
	glUniformMatrix4fv(
		shader->uniformLocations[PROJECTION], 
		1, GL_FALSE, &proj.a);
	GL_ERR("uProjection uniform");
	
	// send camera position 
	glUniform3fv(
		shader->uniformLocations[CAMERA_POSITION], 
		1, mat4ExPosition(&camTrans));
	GL_ERR("uCameraPos uniform");
	
	// send sun direction
	glUniform3fv(
		shader->uniformLocations[SUN_DIRECTION], 
		1, mat4ExForward(&atmTrans));
	GL_ERR("uSunDir uniform");
	
	// send sun color 
	glUniform3fv(
		shader->uniformLocations[SUN_COLOR], 
		1, &atmInfo->sun.r);
	GL_ERR("uSunCol uniform");
	
	// send ambient color 
	glUniform3fv(
		shader->uniformLocations[AMBIENT_COLOR], 
		1, &atmInfo->ambient.r);
	GL_ERR("uAmbientCol uniform");

	// send diffuse color
	glUniform3fv(
		shader->uniformLocations[DIFFUSE_COLOR], 
		1, &material->diffuseCol.r);
	GL_ERR("uDiffuseCol uniform");
	
	// send diffuse texture
	if(material->diffuseMap) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, material->diffuseMap->tex);
		glUniform1i(
			shader->uniformLocations[DIFFUSE_MAP],
			0
		);
	}
	glUniform1i(
		shader->uniformLocations[HAS_DIFFUSE_MAP],
		material->diffuseMap ? 1 : 0	
	);

	// send specular color
	glUniform3fv(
		shader->uniformLocations[SPECULAR_COLOR], 
		1, &material->specularCol.r);
	GL_ERR("uSpecularCol uniform");
	
	// send specular texture
	if(material->specularMap) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, material->specularMap->tex);
		glUniform1i(
			shader->uniformLocations[SPECULAR_MAP],
			1
		);
	}
	glUniform1i(
		shader->uniformLocations[HAS_SPECULAR_MAP],
		material->specularMap ? 1 : 0	
	);
	
	// send shininess 
	glUniform1fv(
		shader->uniformLocations[SHININESS], 
		1, &material->shininess);
	GL_ERR("uSpecularCol uniform");

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
