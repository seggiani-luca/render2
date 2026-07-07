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
	void* val
) {
	inspAction act = NONE;

	// push panel
	quadGui(ctx, SCROLL, (float4){
		1 PAD, 1 PAD,
		WIN - 2 PAD, ROW + 2 PAD
	}, BG_LIGHT);

	// push icon
	iconGui(ctx, SCROLL, (float2){
		2 PAD, 2 PAD + HPAD
	}, ico);

	// push name
	textGui(ctx, SCROLL, (float2){
		3 PAD + ICO_SIZ, 2 PAD + HPAD
	}, name);

	// push edit box
	gui(ctx, SCROLL, (float4){
		2 PAD, 3 PAD + HROW,
		WIN - 4 PAD, HROW - 1 PAD
	}, val);

	// push delete button
	if(buttonGui(ctx, SCROLL, (float4){
		WIN - 4 PAD - ICO_SIZ, 2 PAD,
		 2 PAD + ICO_SIZ, TXT_HEIGHT + 2 PAD
	}, ICO_DELETE, "")) {
		act = DELETE;
	}

	downGui(ctx, SCROLL, ROW + 3 PAD);

	return act;
}

int intFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_INT,
		intGui,
		&((intField*)f)->val
	);
}

int floatFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_FLOAT,
		floatGui,
		&((floatField*)f)->val
	);
}

int stringFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_STRING,
		stringGui,
		&((stringField*)f)->str
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
		&((float2Field*)f)->val
	);
}

int float3FieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_FLOAT3,
		float3Gui,
		&((float2Field*)f)->val
	);
}

int float4FieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_FLOAT4,
		float4Gui,
		&((float2Field*)f)->val
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
		field*(*ctor)(const char* name);
	} fieldButton;
	static const fieldButton fieldButtons[] = {
		{ "New Int",    ICO_INT,    intNew    },
		{ "New Float",  ICO_FLOAT,  floatNew  },
		{ "New String", ICO_STRING, stringNew },
		{ "New Float2", ICO_FLOAT2, float2New },
		{ "New Float3", ICO_FLOAT3, float3New },
		{ "New Float4", ICO_FLOAT4, float4New }
	};
	int fieldButtonCount = (int)(sizeof(fieldButtons) / sizeof(fieldButton));

	// scroll field button layer
	float scrollRange =
		  (TXT_HEIGHT + 3 PAD) * fieldButtonCount // buttons
		+ (TXT_HEIGHT + 3 PAD)                    // name edit box
		- HEIG + 1 PAD;
	scrollGui(ctx, SCROLL, -scrollRange, 0.0f);

	for(int i = 0; i < fieldButtonCount; i++) {
		const fieldButton* f = &fieldButtons[i];

		// push new field button
		if(buttonGui(ctx, SCROLL, (float4){
			1 PAD, 1 PAD,
			WIN - 2 PAD, TXT_HEIGHT + 2 PAD
		}, f->ico, f->label)) {
			// actually construct and append field
			appendField(ent, f->ctor(name));

			// should close
			glfwSetWindowShouldClose(win->gl, 1);
		}

		downGui(ctx, SCROLL, TXT_HEIGHT + 3 PAD);
	}

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
	float scrollRange = 
		  (ent ? (ROW + 3 PAD) * ent->fieldCount : 0) // fields
		+ (TXT_HEIGHT + 3 PAD)                        // entity label
		+ (TXT_HEIGHT + 3 PAD)                        // add field button
		- HEIG + 1 PAD;
	scrollGui(ctx, SCROLL, -scrollRange, 0.0f);

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
