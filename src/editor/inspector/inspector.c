#include "inspector.h"
#include "../editor.h"
#include "../selector/selector.h"
#include "../widget/widget.h"
#include "../../scene/scene.h"
#include "../../exception/exception.h"
#include "../../script/script.h"
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
	char name[GUI_IN_BUF_SIZ];
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
	guiRect ico,
	int (*gui)(guiContext* ctx, guiLayerId id, guiRect rect, void* val),
	void* val,
	int rows
) {
	inspAction act = NONE;

	// push panel
	float height = GUI_ROW + 2 GUI_PAD;
	float bigHeight = GUI_HROW + GUI_HROW * rows + 2 GUI_PAD;
	if(bigHeight > height) height = bigHeight;
	guiPushQuad(ctx, SCROLL, (guiRect){
		1 GUI_PAD, 1 GUI_PAD,
		WIN_WIDTH - 2 GUI_PAD, height
	}, GUI_BG_LIGHT);

	// push icon
	guiIcon(ctx, SCROLL, (guiPos){
		2 GUI_PAD, 2 GUI_PAD + GUI_HPAD
	}, ico);

	// push name
	guiSeparator(ctx, SCROLL, (guiRect){
		3 GUI_PAD + GUI_ICO_SIZ, 2 GUI_PAD + GUI_HPAD,
		WIN_WIDTH - 8 GUI_PAD - 2 * GUI_ICO_SIZ, GUI_TXT_HEIGHT
	}, name);

	// push delete button
	if(guiButton(ctx, SCROLL, (guiRect){
		WIN_WIDTH - 4 GUI_PAD - GUI_ICO_SIZ, 2 GUI_PAD,
		 2 GUI_PAD + GUI_ICO_SIZ, GUI_TXT_HEIGHT + 2 GUI_PAD
	}, ICO_DELETE, "")) {
		act = DELETE;
	}

	// push edit box
	float vPos = ctx->layers[SCROLL].vPos;
	float lastHeight = ctx->layers[SCROLL].height;
	int ret = gui(ctx, SCROLL, (guiRect){
		2 GUI_PAD, 3 GUI_PAD + GUI_HROW,
		WIN_WIDTH - 4 GUI_PAD, GUI_HROW - 1 GUI_PAD
	}, val);
	ctx->layers[SCROLL].vPos = vPos;
	ctx->layers[SCROLL].height = lastHeight;

	guiDown(ctx, SCROLL, height + 1 GUI_PAD);

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

int atmosphereFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_ATMOS,
		atmosphereGui,
		&((atmosphereField*)f)->val,
		7
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

int scriptFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_SCRIPT,
		scriptGui,
		&((scriptField*)f)->ref,
		1
	);
}

int shaderFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_SHADER,
		shaderGui,
		&((shaderField*)f)->ref,
		1
	);
}

// -- entities

// renders the add field GUI
void addFieldGui(window* win) {
	// get context
	addFieldGuiContext* eCtx = (addFieldGuiContext*)guiInit(win);
	if(!eCtx) {
		logEvent(WARN, GUI, guiError());
		return;
	}
	guiContext* ctx = &eCtx->gui;
	entity* ent = eCtx->ent;
	char* name = eCtx->name;

	// update input state
	guiInput(win);

	// push background
	guiPushQuad(ctx, BACKGROUND, (guiRect){
		0, 0, WIN_WIDTH, WIN_HEIGHT 
	}, GUI_BG_ABS);

	// push name edit box
	{
		// mask
		guiPushQuad(ctx, FIXED, (guiRect){
			0, 0,
			WIN_WIDTH, GUI_TXT_HEIGHT + 4 GUI_PAD
		}, GUI_BG_ABS);

		// push label
		guiText(ctx, FIXED, (guiPos){
			1 GUI_PAD, 2 GUI_PAD
		}, "Name:");

		// push name edit box
		stringGui(ctx, FIXED, (guiRect){
			1 GUI_PAD + NEW_OFF, 1 GUI_PAD,
			WIN_WIDTH - 2 GUI_PAD - NEW_OFF, GUI_TXT_HEIGHT + 2 GUI_PAD
		}, name);
	}
	guiDown(ctx, SCROLL, GUI_TXT_HEIGHT + 3 GUI_PAD);

	// declare field button table
	typedef struct {
		const char* label;
		guiRect ico;
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
		{ "New 4x4 Matrix", ICO_MAT4,    mat4New       },

		{ "New Script",     ICO_SCRIPT,  scriptNew     },
		{ "New Shader",     ICO_SHADER,  shaderNew     }
	};
	int fieldButtonCount = (int)(sizeof(fieldButtons) / sizeof(fieldButton));

	for(int i = 0; i < fieldButtonCount; i++) {
		const fieldButton* f = &fieldButtons[i];
		int side = i % 2;

		// push new field button
		if(guiButton(ctx, SCROLL, (guiRect){
			1 GUI_PAD + (HWIN_WIDTH - GUI_HPAD) * side, 1 GUI_PAD,
			HWIN_WIDTH - 1 GUI_PAD - GUI_HPAD, GUI_TXT_HEIGHT + 2 GUI_PAD
		}, f->ico, f->label)) {
			// actually construct and append field
			appendField(ent, f->ctor(name));

			// flag dirty
			scene* scn = ownerScene(ent);
			scn->dirty = 1;

			// should close
			glfwSetWindowShouldClose(win->gl, 1);
		}

		if(side || i == fieldButtonCount - 1) guiDown(ctx, SCROLL, GUI_TXT_HEIGHT + 3 GUI_PAD);
	}

	guiDown(ctx, SCROLL, 1 GUI_PAD);
	guiTrim(ctx, SCROLL);

	// flush changes
	guiFlush(ctx);
}

// gets a callback object for a "new field" menu
winRenderCallback makeAddFieldCallback(entity* ent) {
	// initialize context
	addFieldGuiContext* eCtx = xmalloc(sizeof(addFieldGuiContext));
	eCtx->gui.win = NULL;
	eCtx->gui.child = NULL;
	eCtx->ent = ent;
	*eCtx->name = '\0';

	// return callback
	return (winRenderCallback){
		addFieldGui,
		eCtx,
		guiFree
	};
}

// renders the inspector GUI
void entityGui(window* win) {
	// get context
	entityGuiContext* eCtx = (entityGuiContext*)guiInit(win);
	if(!eCtx) {
		logEvent(WARN, GUI, guiError());
		return;
	}
	guiContext* ctx = &eCtx->gui;
	entity* ent = eCtx->ent;

	// update input state
	guiInput(win);

	// push background
	guiPushQuad(ctx, BACKGROUND, (guiRect){
		0, 0,
		WIN_WIDTH, WIN_HEIGHT 
	}, GUI_BG_ABS);

	// push entity label
	{
		// mask
		guiPushQuad(ctx, FIXED, (guiRect){
			0, 0,
			WIN_WIDTH, GUI_TXT_HEIGHT + 4 GUI_PAD
		}, GUI_BG_ABS);

		// push icon
		guiIcon(ctx, FIXED, (guiPos){
			2 GUI_PAD, 2 GUI_PAD
		}, ICO_ENTITY);

		// push name edit box
		stringGui(ctx, FIXED, (guiRect){
			3 GUI_PAD + GUI_ICO_SIZ, 1 GUI_PAD,
			WIN_WIDTH - 7 GUI_PAD - 2 * GUI_ICO_SIZ, GUI_TXT_HEIGHT + 2 GUI_PAD
		}, ent ? ent->name : NULL);

		// push save button
		if(guiButton(ctx, FIXED, (guiRect){
			WIN_WIDTH - 3 GUI_PAD - GUI_ICO_SIZ, 1 GUI_PAD,
			2 GUI_PAD + GUI_ICO_SIZ, GUI_TXT_HEIGHT + 2 GUI_PAD
		}, ICO_SAVE, "")) {
			// save entity
			serializeEntityFile(ent, getEntityPath(ent->name));
			return; // early quit
		}
	}
	guiDown(ctx, SCROLL, GUI_TXT_HEIGHT + 3 GUI_PAD);

	// scroll field layer
	guiScroll(ctx, SCROLL);

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
				// if script, start
				if(f->vtable == &scriptFieldVtable) {
					scriptStart(ent);
				}

				// flag dirty
				scene* scn = ownerScene(ent);
				scn->dirty = 1;
			} break;
			default: break;
		}

		f = tmp;
	}

	// push new field button
	if(guiButton(ctx, SCROLL, (guiRect){
		1 GUI_PAD, 1 GUI_PAD,
		WIN_WIDTH - 1 GUI_PAD, GUI_TXT_HEIGHT + 2 GUI_PAD
	}, ICO_NEW, "Field") && ent) {
		// create add field window
		guiChildWindow(ctx, winNew(
			ADD_FIELD_WIDTH,
			ADD_FIELD_HEIGHT,
			"Append Field",
			makeAddFieldCallback(ent),
			NULL,
			WIN_DEPTH_OFF
		));
	}
	guiDown(ctx, SCROLL, GUI_TXT_HEIGHT + 4 GUI_PAD);

	// flush changes
	guiFlush(ctx);
}

winRenderCallback makeEntityCallback(entity* ent) {
	// initialize context
	entityGuiContext* eCtx = xmalloc(sizeof(entityGuiContext));
	eCtx->gui.win = NULL;
	eCtx->gui.child = NULL;
	eCtx->ent = ent;

	// return callback
	return (winRenderCallback){
		entityGui,
		eCtx,
		guiFree
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
