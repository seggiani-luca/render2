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
} newFieldGuiContext;

// -- field primitives 

// offset for delete button
#define DEL_OFF 70.0f

// offset for new field button 
#define NEW_OFF 50.0f

void intFieldGui(const field* f, guiContext* ctx) {
	// push panel 
	quadGui(ctx, (float4) {
		PAD, PAD, ctx->win->width - PAD * 2, ROW 
	}, BG_LIGHT);

	// push icon
	iconGui(ctx, (float2) {
		PAD * 2, PAD * 2.5f
	}, ICO_INT);

	// push name
	textGui(ctx, (float2) {
		PAD * 3 + ICO_SIZ, PAD * 2.5f
	}, f->name);

	// push edit box
	intGui(ctx, (float4) {
		PAD * 2, ROW / 2 + PAD, 
		ctx->win->width - PAD * 4, ROW / 2 - PAD
	}, &((intField*)f)->val);

	// push delete button
	if(buttonGui(ctx, (float4) {
		ctx->win->width / 2.0f + DEL_OFF, PAD * 2,
		ctx->win->width / 2.0f - PAD * 2 - DEL_OFF, TXT_HEIGHT + 2 * PAD - 0.5f
	}, ICO_DEL, "Delete")) {
		removeField(((entityGuiContext*)ctx)->ent, f->name);	
	}

	ctx->vPos += ROW + PAD;
}

void floatFieldGui(const field* f, guiContext* ctx) {
	// push panel 
	quadGui(ctx, (float4) {
		PAD, PAD, ctx->win->width - PAD * 2, ROW 
	}, BG_LIGHT);

	// push icon
	iconGui(ctx, (float2) {
		PAD * 2, PAD * 2.5f
	}, ICO_INT);

	// push name
	textGui(ctx, (float2) {
		PAD * 3 + ICO_SIZ, PAD * 2.5f
	}, f->name);

	// push edit box
	floatGui(ctx, (float4) {
		PAD * 2, ROW / 2 + PAD, 
		ctx->win->width - PAD * 4, ROW / 2 - PAD
	}, &((floatField*)f)->val);
	
	// push delete button
	if(buttonGui(ctx, (float4) {
		ctx->win->width / 2.0f + DEL_OFF, PAD * 2,
		ctx->win->width / 2.0f - PAD * 2 - DEL_OFF, TXT_HEIGHT + 2 * PAD - 0.5f
	}, ICO_DEL, "Delete")) {
		removeField(((entityGuiContext*)ctx)->ent, f->name);	
	}

	ctx->vPos += ROW + PAD;
}

void stringFieldGui(const field* f, guiContext* ctx) {
	// push panel 
	quadGui(ctx, (float4) {
		PAD, PAD, ctx->win->width - PAD * 2, ROW 
	}, BG_LIGHT);

	// push icon
	iconGui(ctx, (float2) {
		PAD * 2, PAD * 2.5f
	}, ICO_STR);

	// push name
	textGui(ctx, (float2) {
		PAD * 3 + ICO_SIZ, PAD * 2.5f
	}, f->name);

	// push edit box
	stringGui(ctx, (float4) {
		PAD * 2, ROW / 2 + PAD, 
		ctx->win->width - PAD * 4, ROW / 2 - PAD
	}, ((stringField*)f)->str);
	
	// push delete button
	if(buttonGui(ctx, (float4) {
		ctx->win->width / 2.0f + DEL_OFF, PAD * 2,
		ctx->win->width / 2.0f - PAD * 2 - DEL_OFF, TXT_HEIGHT + 2 * PAD - 0.5f
	}, ICO_DEL, "Delete")) {
		removeField(((entityGuiContext*)ctx)->ent, f->name);	
	}

	ctx->vPos += ROW + PAD;
}

// -- entities

// renders a "new field" menu 
void addFieldGui(window* win) {
	// get context
	newFieldGuiContext* eCtx = (newFieldGuiContext*) initGui(win);
	guiContext* ctx = &eCtx->gui;
	entity* ent = eCtx->ent;
	char* name = eCtx->name;
	
	// update input state
	inputGui(win);

	// push name edit box
	textGui(ctx, (float2){PAD, PAD}, "Name:");
	stringGui(ctx, (float4) {
		PAD + NEW_OFF, PAD,
		win->width - PAD * 2 - NEW_OFF, TXT_HEIGHT + 2 * PAD
	}, name);
	ctx->vPos += TXT_HEIGHT + 3 * PAD;

	// push new int button
	if(buttonGui(ctx, (float4) {
		PAD, PAD,
		win->width - PAD * 2, TXT_HEIGHT + 2 * PAD
	}, ICO_INT, "New Int")) {
		appendField(ent, intNew(name, 0));
		glfwSetWindowShouldClose(win->gl, 1);
	}
	ctx->vPos += TXT_HEIGHT + 3 * PAD;
	
	// push new float button
	if(buttonGui(ctx, (float4) {
		PAD, PAD,
		win->width - PAD * 2, TXT_HEIGHT + 2 * PAD
	}, ICO_FLOAT, "New Float")) {
		appendField(ent, floatNew(name, 0));
		glfwSetWindowShouldClose(win->gl, 1);
	}
	ctx->vPos += TXT_HEIGHT + 3 * PAD;
	
	// push new string button
	if(buttonGui(ctx, (float4) {
		PAD, PAD,
		win->width - PAD * 2, TXT_HEIGHT + 2 * PAD
	}, ICO_STR, "New String")) {
		appendField(ent, stringNew(name, ""));
		glfwSetWindowShouldClose(win->gl, 1);
	}
	ctx->vPos += TXT_HEIGHT + 3 * PAD;
	
	// flush changes
  	flushGui(ctx);
}

// gets a callback object for a "new field" menu 
renderCallback makeAddFieldCallback(entity* ent) {
	// initialize context
	newFieldGuiContext* eCtx = malloc(sizeof(newFieldGuiContext));
	eCtx->gui.win = NULL;
	eCtx->gui.child = NULL;
	eCtx->ent = ent;
	*eCtx->name = '\0';

	// return cback
	return (renderCallback) {
		addFieldGui,
		eCtx,
		freeGui
	};
}

// renders an entity on a GUI
void entityGui(window* win) {
	// get context
	entityGuiContext* eCtx = (entityGuiContext*) initGui(win);
	guiContext* ctx = &eCtx->gui;
	entity* ent = eCtx->ent;

	// update input state
	inputGui(win);

	// push background
	quadGui(ctx, (float4) {
		0.0f, 0.0f, win->width, win->height 
	}, BG_ABS);
	
	// push entity label
	{
		// push icon
		iconGui(ctx, (float2) {
			PAD * 2, PAD * 2
		}, ICO_ENTITY);

		// push text 
		textGui(ctx, (float2) {
			PAD * 3 + ICO_SIZ, PAD * 2
		}, ent->name);
	}
	ctx->vPos += PAD * 3 + TXT_HEIGHT;

	// go through fields, pushing to gui
	field* f = ent->root;
	while(f) {
		guiField(f, ctx);
		f = f->next;
	}

	// push new button
	subWindowGui(ctx,
		INSPECTOR_WIDTH,
		NEW_FIELD_HEIGHT,
		"New Attribute",
		makeAddFieldCallback(ent)	
		, (float4) {
			PAD, PAD, ctx->win->width - PAD * 2, TXT_HEIGHT + 2 * PAD 
	}, ICO_NEW, "New Attribute");

	// flush changes
  	flushGui(ctx);
}

renderCallback makeEntityCallback(entity* ent) {
	// initialize context
	entityGuiContext* eCtx = malloc(sizeof(entityGuiContext));
	eCtx->gui.win = NULL;
	eCtx->gui.child = NULL;
	eCtx->ent = ent;

	// return cback
	return (renderCallback) {
		entityGui,
		eCtx,
		freeGui
	};
}
