#include "inspector.h"
#include "../widget/widget.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <string.h>

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
	DELETE = 1
} inspAction;

// -- field primitives

// primitive for singleton field GUIs
int fieldGui(
	guiContext* ctx,
	const char* name,
	float4 ico,
	void (*gui)(guiContext* ctx, guiLayerId id, float4 rect, void* val),
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
	gui(ctx, SCROLL, (float4){
		2 PAD, 3 PAD + HROW,
		WIN - 4 PAD, HROW - 1 PAD
	}, val);
	ctx->layers[SCROLL].vPos = vPos;
	ctx->layers[SCROLL].height = lastHeight;

	downGui(ctx, SCROLL, height + 1 PAD);

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

// pushes a vector edit box 
void vectorGui(
	guiContext* ctx,
	guiLayerId layId,
	float4 rect,
	void* val,
	int n
) {
	// calculate float edit box span
	float span = (rect.z - (n - 1) * 1 PAD) / n;
	rect.z = span;

	// make float edit boxes
	for(int i = 0; i < n; i++) {
		floatGui(ctx, layId, rect, val + i * sizeof(float));
		rect.x += span + 1 PAD;
	}
}

// macro for vector edit boxes
#define VEC_FIELD_GUI(n)                                                            \
	void float##n##Gui(guiContext* ctx, guiLayerId layId, float4 rect, void* val) { \
	    vectorGui(ctx, layId, rect, val, n);                                        \
	}

// 2D vector edit box
VEC_FIELD_GUI(2)

// 3D vector edit box
VEC_FIELD_GUI(3)

// 4D vector edit box
VEC_FIELD_GUI(4)

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

// pushes a matrix row edit box 
void matrixRowGui(
	guiContext* ctx,
	guiLayerId layId,
	float4 rect,
	void* val,
	int n
) {
	// calculate float edit box span
	float span = (rect.z - (n - 1) * 1 PAD) / n;
	rect.z = span;

	// make float edit boxes
	for(int i = 0; i < n; i++) {
		floatGui(ctx, layId, rect, val + n * i * sizeof(float));
		rect.x += span + 1 PAD;
	}
}

// macro for matrix edit boxes
#define MAT_FIELD_GUI(n)                                                            \
	void mat##n##Gui(guiContext* ctx, guiLayerId layId, float4 rect, void* val) {   \
	    for(int i = 0; i < n; i++) {                                                \
	        matrixRowGui(ctx, layId, rect, ((float*)val) + i, n);                   \
	        downGui(ctx, layId, rect.w + 1 PAD);                                    \
	    }                                                                           \
	}                                                                               \

// 2x2 matrix edit box
MAT_FIELD_GUI(2)

// 3x3 matrix edit box
MAT_FIELD_GUI(3)

// 4x4 matrix edit box
MAT_FIELD_GUI(4)

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

#define TRANS_OFF 84.0f
void transformGui(guiContext* ctx, guiLayerId layId, float4 rect, void* val) {
	rect.x += TRANS_OFF;
	rect.z -= TRANS_OFF;

	// position
	float3Gui(ctx, layId, rect, ((float3*)val));
	downGui(ctx, layId, rect.w + 1 PAD);
	textGui(ctx, SCROLL, (float2){
		2 PAD, 3 PAD
	}, "Position");

	// rotation
	float3Gui(ctx, layId, rect, ((float3*)val) + 1);
	downGui(ctx, layId, rect.w + 1 PAD);
	textGui(ctx, SCROLL, (float2){
		2 PAD, 3 PAD 
	}, "Rotation");

	// scale
	float3Gui(ctx, layId, rect, ((float3*)val) + 2);
	downGui(ctx, layId, rect.w + 1 PAD);
	textGui(ctx, SCROLL, (float2){
		2 PAD, 3 PAD 
	}, "Scale");
}

int transformFieldGui(const field* f, guiContext* ctx) {
	transform* trans = &((transformField*)f)->val;
	
	// get euler angles
	float3 euler = quatToEuler(trans->rotation);

	// get editable matrix
	float3 mat[3] = {
		trans->position,
		euler,
		trans->scale
	};

	// make edit box
	int ret = fieldGui(
		ctx,
		f->name,
		ICO_TRANS,
		transformGui,
		mat,
		3
	);

	// copy values over
	trans->position = mat[0];
	trans->scale = mat[2];
	
	// copy rotation only if changed
	if(mat[1].x != euler.x
	|| mat[1].y != euler.y
	|| mat[1].z != euler.z) {
		trans->rotation = eulerToQuat(mat[1]);
	}

	return ret;
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
		field*(*ctor)(const char* name);
	} fieldButton;
	static const fieldButton fieldButtons[] = {
		{ "New Transform",  ICO_TRANS,   transformNew },
		{ "New Int",        ICO_INT,     intNew       },
		{ "New Float",      ICO_FLOAT,   floatNew     },
		{ "New String",     ICO_STRING,  stringNew    },
		{ "New 2D Vector",  ICO_FLOAT2,  float2New    },
		{ "New 3D Vector",  ICO_FLOAT3,  float3New    },
		{ "New 4D Vector",  ICO_FLOAT4,  float4New    },
		{ "New 2x2 Matrix", ICO_MAT2,    mat2New      },
		{ "New 3x3 Matrix", ICO_MAT3,    mat3New      },
		{ "New 4x4 Matrix", ICO_MAT4,    mat4New      },
		{ "New Quaternion", ICO_QUAT,    quatNew      }
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
	},BG_ABS);

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
			makeAddFieldCallback(ent)
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
}

entity* getEntityCallback(window* win) {
	entityGuiContext* ctx = (entityGuiContext*)win->cbak.ctx;
	return ctx->ent;
}
