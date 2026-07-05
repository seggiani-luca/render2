#include "inspector.h"
#include "../widget/widget.h"
#include <stdlib.h>
#include <string.h>

// -- field primitives 

void intFieldGui(const field* f, guiContext* ctx) {
	// push panel 
	quadGui(ctx, (float4) {
		PAD, PAD, ctx->win->width - PAD * 2, ROW 
	}, BG_LIGHT);

	// push icon
	iconGui(ctx, (float2) {
		PAD * 2, PAD * 2
	}, ICO_INT);

	// push name
	textGui(ctx, (float2) {
		PAD * 3 + ICO_SIZ, PAD * 2
	}, f->name);

	// push edit box
	intGui(ctx, (float4) {
		PAD * 2, ROW / 2 + PAD, 
		ctx->win->width - PAD * 4, ROW / 2 - PAD
	}, &((intField*)f)->val);

	ctx->vPos += ROW + PAD;
}

void floatFieldGui(const field* f, guiContext* ctx) {
	// push panel 
	quadGui(ctx, (float4) {
		PAD, PAD, ctx->win->width - PAD * 2, ROW 
	}, BG_LIGHT);

	// push icon
	iconGui(ctx, (float2) {
		PAD * 2, PAD * 2
	}, ICO_FLOAT);

	// push name
	textGui(ctx, (float2) {
		PAD * 3 + ICO_SIZ, PAD * 2
	}, f->name);

	// push edit box
	floatGui(ctx, (float4) {
		PAD * 2, ROW / 2 + PAD, 
		ctx->win->width - PAD * 4, ROW / 2 - PAD
	}, &((floatField*)f)->val);

	ctx->vPos += ROW + PAD;
}

void stringFieldGui(const field* f, guiContext* ctx) {
	// push panel 
	quadGui(ctx, (float4) {
		PAD, PAD, ctx->win->width - PAD * 2, ROW 
	}, BG_LIGHT);

	// push icon
	iconGui(ctx, (float2) {
		PAD * 2, PAD * 2
	}, ICO_STR);

	// push name
	textGui(ctx, (float2) {
		PAD * 3 + ICO_SIZ, PAD * 2
	}, f->name);

	// push edit box
	stringGui(ctx, (float4) {
		PAD * 2, ROW / 2 + PAD, 
		ctx->win->width - PAD * 4, ROW / 2 - PAD
	}, ((stringField*)f)->str);

	ctx->vPos += ROW + PAD;
}

// -- entities

// context for entity GUI callback
typedef struct {
	// GUI context
	guiContext gui;

	// entity
	entity* ent;
} entityGuiContext;

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
	}, BG_DARK);

	// push icon
	iconGui(ctx, (float2) {
		PAD * 2, PAD * 2
	}, ICO_ENTITY);

	// push text 
	textGui(ctx, (float2) {
		PAD * 3 + ICO_SIZ, PAD * 2
	}, ent->name);

	ctx->vPos += PAD * 3 + TXT_HEIGHT;

	// go through fields, pushing to gui
	field* f = ent->root;
	while(f) {
		guiField(f, ctx);
		f = f->next;
	}

	// flush changes
  	flushGui(ctx);
}

renderCallback makeEntityCallback(entity* ent) {
	// initialize context
	entityGuiContext* eCtx = malloc(sizeof(entityGuiContext));
	eCtx->gui.win = NULL;
	eCtx->ent = ent;

	// return cback
	return (renderCallback) {
		entityGui,
		eCtx,
		freeGui
	};
}
