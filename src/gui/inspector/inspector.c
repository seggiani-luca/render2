#include "inspector.h"
#include "../widget/widget.h"
#include "../../scene/scene.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <string.h>

// -- contexts

// context for entity GUI callback
typedef struct {
	// GUI context
	guiContext gui;

	// entity
	entity* ent;
} entityGuiContext;

// context for new field GUI callback
typedef struct {
	// GUI context
	guiContext gui;

	// entity
	entity* ent;

	// name buffer
	char name[ENT_NAME_SIZ];
} addFieldGuiContext;

// enum for inspector action
typedef enum {
	NONE = 0,
	MODIFY = 1,
	DELETE = 2
} inspAction;

// -- field primitives

// primitive for singleton field GUIs
int fieldGui(
	guiContext* ctx,
	const char* name,
	float4 ico,
	int (*gui)(guiContext* ctx, guiLayerId id, float4 rect, void* val),
	void* val,
	int rows
) {
	inspAction act = NONE;

	// push panel
	float height = ROW + 2 PAD;
	float bigHeight = HROW + HROW * rows + 2 PAD;
	if(bigHeight > height) height = bigHeight;
	quadGui(ctx, SCROLL, (float4){
		1 PAD, 1 PAD,
		WIN - 2 PAD, height
	}, BG_LIGHT);

	// push icon
	iconGui(ctx, SCROLL, (float2){
		2 PAD, 2 PAD + HPAD
	}, ico);

	// push name
	textGui(ctx, SCROLL, (float2){
		3 PAD + ICO_SIZ, 2 PAD + HPAD
	}, name);

	// push delete button
	if(buttonGui(ctx, SCROLL, (float4){
		WIN - 4 PAD - ICO_SIZ, 2 PAD,
		 2 PAD + ICO_SIZ, TXT_HEIGHT + 2 PAD
	}, ICO_DELETE, "")) {
		act = DELETE;
	}

	// push edit box
	float vPos = ctx->layers[SCROLL].vPos;
	float lastHeight = ctx->layers[SCROLL].height;
	int ret = gui(ctx, SCROLL, (float4){
		2 PAD, 3 PAD + HROW,
		WIN - 4 PAD, HROW - 1 PAD
	}, val);
	ctx->layers[SCROLL].vPos = vPos;
	ctx->layers[SCROLL].height = lastHeight;

	downGui(ctx, SCROLL, height + 1 PAD);

	// signal if modified
	if(ret) act = MODIFY;

	return act;
}

int intFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_INT,
		intGui,
		&((intField*)f)->val,
		1
	);
}

int floatFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_FLOAT,
		floatGui,
		&((floatField*)f)->val,
		1
	);
}

int stringFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_STRING,
		stringGui,
		&((stringField*)f)->str,
		1
	);
}

int float2FieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_FLOAT2,
		float2Gui,
		&((float2Field*)f)->val,
		1
	);
}

int float3FieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_FLOAT3,
		float3Gui,
		&((float2Field*)f)->val,
		1
	);
}

int float4FieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_FLOAT4,
		float4Gui,
		&((float2Field*)f)->val,
		1
	);
}

int mat2FieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_MAT2,
		mat2Gui,
		&((mat2Field*)f)->val,
		2
	);
}

int mat3FieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_MAT3,
		mat3Gui,
		&((mat2Field*)f)->val,
		3
	);
}

int mat4FieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_MAT4,
		mat4Gui,
		&((mat2Field*)f)->val,
		4
	);
}

int quatFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_QUAT,
		float4Gui,
		&((quatField*)f)->val,
		1
	);
}

// pushes a transform edit box
int transformGui(guiContext* ctx, guiLayerId layId, float4 rect, void* val) {
	rect.x += TRANS_OFF;
	rect.z -= TRANS_OFF;

	// modify return
	int ret = 0;

	// position
	if(float3Gui(ctx, layId, rect, ((float3*)val))) ret = 1;
	downGui(ctx, layId, rect.w + 1 PAD);
	textGui(ctx, SCROLL, (float2){
		2 PAD, 3 PAD
	}, "Position");

	// rotation
	if(float3Gui(ctx, layId, rect, ((float3*)val) + 1)) ret = 1;
	downGui(ctx, layId, rect.w + 1 PAD);
	textGui(ctx, SCROLL, (float2){
		2 PAD, 3 PAD 
	}, "Rotation");

	// scale
	if(float3Gui(ctx, layId, rect, ((float3*)val) + 2)) ret = 1;
	downGui(ctx, layId, rect.w + 1 PAD);
	textGui(ctx, SCROLL, (float2){
		2 PAD, 3 PAD 
	}, "Scale");

	return ret;
}

int transformFieldGui(const field* f, guiContext* ctx) {
	transformField* tf = (transformField*)f;
	transform* t = &tf->val;

	// get euler angles
	float3 euler = quatToEuler(t->rotation);

	// get editable matrix
	tf->mat[0] = t->position;
	tf->mat[1] = euler;
	tf->mat[2] = t->scale;

	// make edit box
	int ret = fieldGui(
		ctx,
		f->name,
		ICO_TRANS,
		transformGui,
		tf->mat,
		3
	);

	// copy values over
	t->position = tf->mat[0];
	t->scale = tf->mat[2];

	// copy rotation only if changed
	if(tf->mat[1].x != euler.x
	|| tf->mat[1].y != euler.y
	|| tf->mat[1].z != euler.z) {
		t->rotation = eulerToQuat(tf->mat[1]);
	}

	return ret;
}

// pushes a camera edit box
int cameraGui(guiContext* ctx, guiLayerId layId, float4 rect, void* val) {
	rect.x += TRANS_OFF;
	rect.z -= TRANS_OFF;

	camera* c = (camera*)val;

	// modify return
	int ret = 0;

	// fov
	if(floatGui(ctx, layId, rect, &c->fov)) ret = 1;
	downGui(ctx, layId, rect.w + 1 PAD);
	textGui(ctx, SCROLL, (float2){
		2 PAD, 3 PAD
	}, "Fov");

	// near
	if(floatGui(ctx, layId, rect, &c->nearPlane)) ret = 1;
	downGui(ctx, layId, rect.w + 1 PAD);
	textGui(ctx, SCROLL, (float2){
		2 PAD, 3 PAD 
	}, "Near");

	// far
	if(floatGui(ctx, layId, rect, &c->farPlane)) ret = 1;
	downGui(ctx, layId, rect.w + 1 PAD);
	textGui(ctx, SCROLL, (float2){
		2 PAD, 3 PAD 
	}, "Far");

	return ret;
}

int cameraFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_CAMERA,
		cameraGui,
		&((cameraField*)f)->val,
		3
	);
}

// pushes an atmosphere edit box
int atmosphereGui(guiContext* ctx, guiLayerId layId, float4 rect, void* val) {
	rect.x += TRANS_OFF;
	rect.z -= TRANS_OFF;

	atmosphere* a = (atmosphere*)val;
	
	// modify return
	int ret = 0;

	// ambient
	if(float3Gui(ctx, layId, rect, &a->ambient)) ret = 1;
	downGui(ctx, layId, rect.w + 1 PAD);
	textGui(ctx, SCROLL, (float2){
		2 PAD, 3 PAD
	}, "Ambient");
	
	if(textureGui(ctx, layId, rect, &a->ambientMap)) ret = 1;
	downGui(ctx, layId, rect.w + 1 PAD);
	textGui(ctx, SCROLL, (float2){
		2 PAD, 3 PAD
	}, "Cubemap");
	
	// sun 
	if(float3Gui(ctx, layId, rect, &a->sun)) ret = 1;
	downGui(ctx, layId, rect.w + 1 PAD);
	textGui(ctx, SCROLL, (float2){
		2 PAD, 3 PAD
	}, "Sun");
	
	// background
	if(float3Gui(ctx, layId, rect, &a->background)) ret = 1;
	downGui(ctx, layId, rect.w + 1 PAD);
	textGui(ctx, SCROLL, (float2){
		2 PAD, 3 PAD
	}, "Background");

	return ret;
}

int atmosphereFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_ATMOS,
		atmosphereGui,
		&((atmosphereField*)f)->val,
		4
	);
}

int textureFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_TEX,
		textureGui,
		&((meshField*)f)->ref,
		1
	);
}

int meshFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_MESH,
		meshGui,
		&((meshField*)f)->ref,
		1
	);
}

int materialFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_MAT,
		materialGui,
		&((meshField*)f)->ref,
		1
	);
}

// -- entities

// renders the add field GUI
void addFieldGui(window* win) {
	// get context
	addFieldGuiContext* eCtx = (addFieldGuiContext*)initGui(win);
	guiContext* ctx = &eCtx->gui;
	entity* ent = eCtx->ent;
	char* name = eCtx->name;

	// update input state
	inputGui(win);

	// push background
	quadGui(ctx, BACKGROUND, (float4){
		0, 0, WIN, HEIG 
	}, BG_ABS);

	// push name edit box
	{
		// mask
		quadGui(ctx, FIXED, (float4){
			0, 0,
			WIN, TXT_HEIGHT + 4 PAD
		}, BG_ABS);

		// push label
		textGui(ctx, FIXED, (float2){
			1 PAD, 2 PAD
		}, "Name:");

		// push name edit box
		stringGui(ctx, FIXED, (float4){
			1 PAD + NEW_OFF, 1 PAD,
			WIN - 2 PAD - NEW_OFF, TXT_HEIGHT + 2 PAD
		}, name);
	}
	downGui(ctx, SCROLL, TXT_HEIGHT + 3 PAD);

	// declare field button table
	typedef struct {
		const char* label;
		float4 ico;
		field* (*ctor)(const char* name);
	} fieldButton;
	static const fieldButton fieldButtons[] = {
		{ "New Transform",  ICO_TRANS,   transformNew  },
		{ "New Texture",    ICO_TEX,     textureNew    },

		{ "New Mesh",       ICO_MESH,    meshNew       },
		{ "New Material",   ICO_MAT,     materialNew   },
		
		{ "New Camera",     ICO_CAMERA,  cameraNew     },
		{ "New Atmosphere", ICO_ATMOS,   atmosphereNew },

		{ "New Int",        ICO_INT,     intNew        },
		{ "New Float",      ICO_FLOAT,   floatNew      },

		{ "New String",     ICO_STRING,  stringNew     },
		{ "New Quaternion", ICO_QUAT,    quatNew       },

		{ "New 2D Vector",  ICO_FLOAT2,  float2New     },
		{ "New 3D Vector",  ICO_FLOAT3,  float3New     },

		{ "New 4D Vector",  ICO_FLOAT4,  float4New     },
		{ "New 2x2 Matrix", ICO_MAT2,    mat2New       },

		{ "New 3x3 Matrix", ICO_MAT3,    mat3New       },
		{ "New 4x4 Matrix", ICO_MAT4,    mat4New       }
	};
	int fieldButtonCount = (int)(sizeof(fieldButtons) / sizeof(fieldButton));

	for(int i = 0; i < fieldButtonCount; i++) {
		const fieldButton* f = &fieldButtons[i];
		int side = i % 2;

		// push new field button
		if(buttonGui(ctx, SCROLL, (float4){
			1 PAD + (HWIN - HPAD) * side, 1 PAD,
			HWIN - 1 PAD - HPAD, TXT_HEIGHT + 2 PAD
		}, f->ico, f->label)) {
			// actually construct and append field
			appendField(ent, f->ctor(name));

			// flag dirty
			scene* scn = ownerScene(ent);
			scn->dirty = 1;

			// should close
			glfwSetWindowShouldClose(win->gl, 1);
		}

		if(side || i == fieldButtonCount - 1) downGui(ctx, SCROLL, TXT_HEIGHT + 3 PAD);
	}

	downGui(ctx, SCROLL, 1 PAD);
	trimGui(ctx);

	// flush changes
	flushGui(ctx);
}

// gets a callback object for a "new field" menu
renderCallback makeAddFieldCallback(entity* ent) {
	// initialize context
	addFieldGuiContext* eCtx = malloc(sizeof(addFieldGuiContext));
	eCtx->gui.win = NULL;
	eCtx->gui.child = NULL;
	eCtx->ent = ent;
	*eCtx->name = '\0';

	// return callback
	return (renderCallback){
		addFieldGui,
		eCtx,
		freeGui
	};
}

// renders the inspector GUI
void entityGui(window* win) {
	// get context
	entityGuiContext* eCtx = (entityGuiContext*)initGui(win);
	guiContext* ctx = &eCtx->gui;
	entity* ent = eCtx->ent;

	// update input state
	inputGui(win);

	// push background
	quadGui(ctx, BACKGROUND, (float4){
		0, 0,
		WIN, HEIG 
	}, BG_ABS);

	// push entity label
	{
		// mask
		quadGui(ctx, FIXED, (float4){
			0, 0,
			WIN, TXT_HEIGHT + 4 PAD
		}, BG_ABS);

		// push icon
		iconGui(ctx, FIXED, (float2){
			2 PAD, 2 PAD
		}, ICO_ENTITY);

		// push name edit box
		stringGui(ctx, FIXED, (float4){
			3 PAD + ICO_SIZ, 1 PAD,
			WIN - 4 PAD - ICO_SIZ, TXT_HEIGHT + 2 PAD
		}, ent ? ent->name : NULL);
	}
	downGui(ctx, SCROLL, TXT_HEIGHT + 3 PAD);

	// scroll field layer
	scrollGui(ctx, SCROLL);

	// go through fields, pushing to gui
	field* f = ent ? ent->root : NULL;
	while(f) {
		// advance first
		field* tmp = f->next;

		// get action
		inspAction act = guiField(f, ctx);
		switch(act) {
			case DELETE: {
				removeField(ent, f->name);
				
				// flag dirty
				scene* scn = ownerScene(ent);
				scn->dirty = 1;
			} break;
			case MODIFY: {	
				// flag dirty
				scene* scn = ownerScene(ent);
				scn->dirty = 1;
			} break;
			default: break;
		}

		f = tmp;
	}

	// push new button
	if(buttonGui(ctx, SCROLL, (float4){
		1 PAD, 1 PAD,
		WIN - 2 PAD, TXT_HEIGHT + 2 PAD
	}, ICO_NEW, "Append") && ent) {
		// create add field window
		subWindowGui(ctx, newWindow(
			ADD_FIELD_WIDTH,
			ADD_FIELD_HEIGHT,
			"Append Field",
			makeAddFieldCallback(ent),
			loadIcon(WIN_NEWITEM_ICO),
			0
		));
	}
	downGui(ctx, SCROLL, TXT_HEIGHT + 4 PAD);

	// flush changes
	flushGui(ctx);
}

renderCallback makeEntityCallback(entity* ent) {
	// initialize context
	entityGuiContext* eCtx = malloc(sizeof(entityGuiContext));
	eCtx->gui.win = NULL;
	eCtx->gui.child = NULL;
	eCtx->ent = ent;

	// return callback
	return (renderCallback){
		entityGui,
		eCtx,
		freeGui
	};
}

void changeEntityCallback(window* win, entity* ent) {
	entityGuiContext* ctx = (entityGuiContext*)win->cbak.ctx;
	ctx->ent = ent;

	// reset hot
	ctx->gui.in.hotReset = 1;
}

entity* getEntityCallback(window* win) {
	entityGuiContext* ctx = (entityGuiContext*)win->cbak.ctx;
	return ctx->ent;
}
