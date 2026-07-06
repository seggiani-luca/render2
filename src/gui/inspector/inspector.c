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

// -- field primitives 

// primitive for singleton field GUIs
int fieldGui(
	guiContext* ctx,
	const char* name,
	float4 ico,
	void (*gui)(guiContext* ctx, guiLayerId id, float4 rect, void* val),
	void* val
) {
	// push panel 
	quadGui(ctx, SCROLL, (float4) {
		1 PAD,
		1 PAD,
		WIN - 2 PAD,
		ROW 
	}, BG_LIGHT);

	// push icon
	iconGui(ctx, SCROLL, (float2) {
		2 PAD,
		2 PAD + HPAD
	}, ico);

	// push name
	textGui(ctx, SCROLL, (float2) {
		3 PAD + ICO_SIZ,
		2 PAD + HPAD 
	}, name);

	// push edit box
	gui(ctx, SCROLL, (float4) {
		2 PAD,
		1 PAD + HROW, 
		WIN  - 4 PAD,
		HROW - 1 PAD
	}, val);

	// push delete button
	int del = (buttonGui(ctx, SCROLL, (float4) {
		HWIN + DEL_WIDTH,
		2 PAD ,
		HWIN       - 2 PAD - DEL_WIDTH,
		TXT_HEIGHT + 2 PAD - 0.5f
	}, ICO_DEL, "Delete"));

	downGui(ctx, SCROLL, ROW + 1 PAD);

	return del;
}

int intFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_INT,
		intGui,
		&((intField*) f)->val
	);
}

int floatFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_FLOAT,
		floatGui,
		&((floatField*) f)->val
	);
}

int stringFieldGui(const field* f, guiContext* ctx) {
	return fieldGui(
		ctx,
		f->name,
		ICO_STR,
		stringGui,
		&((stringField*) f)->str
	);
}

// -- entities

// renders the add field GUI 
void addFieldGui(window* win) {
	// get context
	addFieldGuiContext* eCtx = (addFieldGuiContext*) initGui(win);
	guiContext* ctx = &eCtx->gui;
	entity* ent = eCtx->ent;
	char* name = eCtx->name;

	// update input state
	inputGui(win);

	// push background
	quadGui(ctx, BACKGROUND, (float4) {
		0,
		0,
		WIN,
		ADD_FIELD_HEIGHT	
	}, BG_ABS);

	// push name edit box
	{
		// mask
		quadGui(ctx, FIXED, (float4) {
			0,
			0,
			WIN,
			TXT_HEIGHT + 4 PAD 
		}, BG_ABS);

		// push label
		textGui(ctx, FIXED, (float2){
			1 PAD,
			2 PAD
		}, "Name:");

		// push name edit box
		stringGui(ctx, FIXED, (float4) {
			1 PAD + NEW_OFF,
			1 PAD,
			WIN - 2 PAD - NEW_OFF, 
			TXT_HEIGHT + 2 PAD
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
		{ "New Int",    ICO_INT,   intNew    },
		{ "New Float",  ICO_FLOAT, floatNew  },
		{ "New String", ICO_STR,   stringNew }
	};
	int fieldButtonCount = (int) (sizeof(fieldButtons) / sizeof(fieldButton));

	// scroll field button layer
	float scrollRange =
	    (TXT_HEIGHT + 3 PAD) * fieldButtonCount // buttons 
	  + (TXT_HEIGHT + 3 PAD)                    // name edit box 
	  - ADD_FIELD_HEIGHT + 1 PAD;
	scrollGui(ctx, SCROLL, -scrollRange, 0.0f);

	for(int i = 0; i < fieldButtonCount; i++) {
		const fieldButton* f = &fieldButtons[i];

		// push new field button
		if(buttonGui(ctx, SCROLL, (float4) {
			1 PAD, 
			1 PAD,
			WIN        - 2 PAD, 
			TXT_HEIGHT + 2 PAD
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
	return (renderCallback) {
		addFieldGui,
		eCtx,
		freeGui
	};
}

// renders the inspector GUI 
void entityGui(window* win) {
	// get context
	entityGuiContext* eCtx = (entityGuiContext*) initGui(win);
	guiContext* ctx = &eCtx->gui;
	entity* ent = eCtx->ent;

	// update input state
	inputGui(win);

	// push background
	quadGui(ctx, BACKGROUND, (float4) {
		0,
		0,
		WIN,
		INSPECTOR_HEIGHT
	}, BG_ABS);

	// push entity label
	{
		// mask
		quadGui(ctx, FIXED, (float4) {
			0,
			0,
			WIN,
			TXT_HEIGHT + 4 PAD 
		}, BG_ABS);

		// push icon
		iconGui(ctx, FIXED, (float2) {
			2 PAD, 
			2 PAD
		}, ICO_ENTITY);

		// push name edit box
		stringGui(ctx, FIXED, (float4) {
			3 PAD + ICO_SIZ, 
			1 PAD, 
			WIN        - 4 PAD - ICO_SIZ, 
			TXT_HEIGHT + 2 PAD
		}, ent ? ent->name : NULL);
	}
	downGui(ctx, SCROLL, TXT_HEIGHT + 3 PAD);

	// scroll field layer
	float scrollRange =
	    ent ? (ROW + 1 PAD) * ent->fieldCount : 0 // fields
	  + (TXT_HEIGHT + 3 PAD)                      // entity label
	  + (TXT_HEIGHT + 3 PAD)                      // add field button
	  - INSPECTOR_HEIGHT + 1 PAD;
	scrollGui(ctx, SCROLL, -scrollRange, 0.0f);

	// go through fields, pushing to gui
	field* f = ent ? ent->root : NULL;
	while(f) {
		field* tmp = f->next;
		if(guiField(f, ctx)) removeField(ent, f->name);
		f = tmp;
	}

	// push new button
	if(buttonGui(ctx, SCROLL, (float4) {
			1 PAD,
			1 PAD,
			WIN        - 2 PAD, 
			TXT_HEIGHT + 2 PAD 
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
	return (renderCallback) {
		entityGui,
		eCtx,
		freeGui
	};
}

void changeEntityCallback(window* win, entity* ent) {
	entityGuiContext* ctx = (entityGuiContext*) win->cbak.ctx;
	ctx->ent = ent;
}
