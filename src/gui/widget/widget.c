#include "widget.h"
#include "../../data/texture/texture.h"
#include "../../data/mesh/mesh.h"
#include "../../data/material/material.h"
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// -- contexts

// context for data selector GUI callback
typedef struct {
	// GUI context
	guiContext gui;

	// data reference to update 
	dataRef** ref;

	// data table to update from 
	dataTable* tab;

	// path of data to find
	char path[DAT_PATH_SIZ];

	// original GUI context
	guiContext* orig;
} dataselGuiContext;

// -- data

// renders the data selector GUI
void dataselGui(window* win) {
	// get context
	dataselGuiContext* dCtx = (dataselGuiContext*)initGui(win);
	guiContext* ctx = &dCtx->gui;
	dataTable* tab = dCtx->tab;
	char* path = dCtx->path;
	guiContext* orig = dCtx->orig;

	// update input state
	inputGui(win);

	// push background
	quadGui(ctx, BACKGROUND, (float4){
		0, 0,
		WIN, HEIG 
	}, BG_ABS);

	// push bottom frame
	quadGui(ctx, FIXED, (float4) {
		0, HEIG - TXT_HEIGHT - 4 PAD,
		WIN, TXT_HEIGHT + 4 PAD
	}, BG_LIGHT);

	// push label
	textGui(ctx, FIXED, (float2){
		1 PAD, HEIG - TXT_HEIGHT - 2 PAD
	}, "Path:");

	// push path edit box
	stringGui(ctx, FIXED, (float4){
		1 PAD + PATH_OFF, HEIG - TXT_HEIGHT - 3 PAD,
		IMPORT_OFF - 2 PAD - PATH_OFF, TXT_HEIGHT + 2 PAD
	}, path);

	// push import button
	if(buttonGui(ctx, FIXED, (float4){
		IMPORT_OFF, HEIG - TXT_HEIGHT - 3 PAD,
		WIN - IMPORT_OFF - 1 PAD, TXT_HEIGHT + 2 PAD
	}, ICO_NUFILE, "Import")) {
		// import data
		if((*dCtx->ref)) freeData((*dCtx->ref)->data, tab);
		*dCtx->ref = importData(path, tab);
			
		// update original context
		orig->in.dataSet = 1;

		// should close
		glfwSetWindowShouldClose(win->gl, 1);
	}

	// go through references, pushing to gui
	dataRef* cur = tab->root;
	while(cur) {
		char str[DAT_PATH_SIZ + 16];
		snprintf(str, DAT_PATH_SIZ + 16, "%s (%d refs)", 
				cur->path, cur->refCount);

		if(buttonGui(ctx, SCROLL, (float4){
			1 PAD, 1 PAD,
			WIN - 2 PAD, TXT_HEIGHT + 2 PAD
		}, ICO_FILE, str)) {
			// use this reference
			if((*dCtx->ref)) freeData((*dCtx->ref)->data, tab);
			*dCtx->ref = importData(cur->path, tab);

			// update original context
			orig->in.dataSet = 1;

			// should close
			glfwSetWindowShouldClose(win->gl, 1);
		}

		downGui(ctx, SCROLL, TXT_HEIGHT + 3 PAD);
		cur = cur->next;
	}

	downGui(ctx, SCROLL, 1 PAD);

	// scroll reference layer
	scrollGui(ctx, SCROLL);

	// flush changes
	flushGui(ctx);
}

renderCallback makeDataselCallback(
	dataRef** ref,
	dataTable* tab,
	guiContext* orig
) {
	// initialize context
	dataselGuiContext* dCtx = malloc(sizeof(dataselGuiContext));
	dCtx->gui.win = NULL;
	dCtx->gui.child = NULL;
	dCtx->ref = ref;
	dCtx->tab = tab;
	*dCtx->path = '\0';
	dCtx->orig = orig;

	// return callback
	return (renderCallback){
		dataselGui,
		dCtx,
		freeGui
	};
}

// -- input primitives

int hoverGui(guiContext* ctx, guiLayerId layId, float4 rect) {
	rect.y += ctx->layers[layId].vPos;
	return ctx->in.xCur >= rect.x && ctx->in.xCur <= rect.x + rect.z &&
	       ctx->in.yCur >= rect.y && ctx->in.yCur <= rect.y + rect.w;
}

int relesGui(guiContext* ctx, guiLayerId layId, float4 rect) {
	int reles = hoverGui(ctx, layId, rect) && ctx->in.curReles;

	// consume other input
	if(reles
	|| (hoverGui(ctx, layId, rect) && ctx->in.curPress)) {
		ctx->in.curReles = ctx->in.curPress = ctx->in.curDown = 0;
	}

	return reles;
}

int pressGui(guiContext* ctx, guiLayerId layId, float4 rect) {
	int press = hoverGui(ctx, layId, rect) && ctx->in.curPress;

	// consume other input
	if(press
	|| (hoverGui(ctx, layId, rect) && ctx->in.curReles)) {
		ctx->in.curReles = ctx->in.curPress = ctx->in.curDown = 0;
	}

	return press;
}

char* bufferGui(
	guiContext* ctx,
	guiLayerId layId,
	uint64_t id,
	float4 rect,
	int* submit,
	int* active
) {
	*active = *submit = 0;

	// check if pressed
	int press = pressGui(ctx, layId, rect);

	// activate on press
	if((ctx->in.hotId == 0) && press) {
		// set id
		ctx->in.hotId = id;

		// clear buffer
		memset(ctx->in.keyBuf, 0, IN_BUF_SIZ);
		ctx->in.keyBufSiz = 0;

		return ctx->in.keyBuf;
	}

	// check if active
	if(ctx->in.hotId != id) return NULL;
	*active = 1;

	// discard on outside press or escape
	if((!hoverGui(ctx, layId, rect) && ctx->in.curDown) || ctx->in.escape) {
		// reset id
		ctx->in.hotReset = 1;

		// on outside press, keep it (for ux)
		if(*ctx->in.keyBuf != '\0' && !ctx->in.escape) *submit = 1;

		// return temp. buffer
		return ctx->in.keyBuf;
	}

	// return on complete
	if(ctx->in.enter) {
		// reset id
		ctx->in.hotReset = 1;

		// submit temp. buffer
		*submit = 1;
		return ctx->in.keyBuf;
	}

	// return temp. buffer
	return ctx->in.keyBuf;
}

void scrollGui(guiContext* ctx, guiLayerId layId) {
	float max = 0.0f;
	float min = -ctx->layers[layId].lastHeight
	          + HEIG
	          + ctx->layers[FIXED].lastHeight;

	// get absolute scroll
	ctx->in.absScroll += ctx->in.scroll * SCROLL_SENS;
	if(ctx->in.absScroll < min) ctx->in.absScroll = min;
	if(ctx->in.absScroll > max) ctx->in.absScroll = max;

	// scroll vPos
	ctx->layers[layId].vPos += ctx->in.absScroll;

	// consume scroll
	ctx->in.scroll = 0.0f;
}

// -- rendering primitives

void downGui(guiContext* ctx, guiLayerId layId, float amt) {
	ctx->layers[layId].vPos += amt;
	ctx->layers[layId].height += amt;
}

void trimGui(guiContext* ctx) {
	window* win = ctx->win;
	resizeWindow(win, win->width, ctx->layers[SCROLL].vPos);
}

void quadGui(guiContext* ctx, guiLayerId layId, float4 rect, float4 uv) {
	pushGui(&ctx->layers[layId], (quad){
		rect.x, rect.y + ctx->layers[layId].vPos, 
		rect.z, rect.w,
		uv
	});
}

void borderGui(guiContext* ctx, guiLayerId layId, float4 rect, float4 uv) {
	// i hate high DPI displays
	float border = fbToWinH(ctx->win, BORDER);

	quadGui(ctx, layId, (float4){rect.x,                   rect.y,                   rect.z, border}, uv);
	quadGui(ctx, layId, (float4){rect.x,                   rect.y + rect.w - border, rect.z, border}, uv);
	quadGui(ctx, layId, (float4){rect.x,                   rect.y,                   border, rect.w}, uv);
	quadGui(ctx, layId, (float4){rect.x + rect.z - border, rect.y,                   border, rect.w}, uv);
}

void textGui(guiContext* ctx, guiLayerId layId, float2 pos, const char* str) {
	char c;
	int w = 0;

	// go through all characters
	while((c = *str++)) {
		// extract character index
		int tX = (c - 32) % 32;
		int tY = (c - 32) / 32;

		// calculate UV
		float4 uv = {
			UV((float)tX / 2, tY),
			UV((float)tX / 2 + 0.5f, tY + 1.0f)
		};

		// push characters
		pushGui(&ctx->layers[layId], (quad){
			pos.x + w, ctx->layers[layId].vPos + pos.y, 
			TXT_WIDTH, TXT_HEIGHT,
			uv
		});

		// move right
		w += TXT_WIDTH;
	}
}

void iconGui(guiContext* ctx, guiLayerId layId, float2 pos, float4 uv) {
	pushGui(&ctx->layers[layId], (quad){
		pos.x, pos.y + ctx->layers[layId].vPos, 
		ICO_SIZ, ICO_SIZ,
		uv
	});
}

int buttonGui(
	guiContext* ctx,
	guiLayerId layId,
	float4 rect,
	float4 ico,
	const char* str
) {
	int press = hoverGui(ctx, layId, rect)
	            && ctx->in.curDown;

	// push quad
	quadGui(ctx, layId, rect, BG_LIGHT);
	borderGui(ctx, layId, rect, FG_DARK);

	// display icon
	iconGui(ctx, layId, (float2){
		rect.x + 1 PAD, rect.y + (press ? 1 PAD : HPAD)
	}, ico);

	// display text
	textGui(ctx, layId, (float2){
		rect.x + 2 PAD + ICO_SIZ, rect.y + 1 PAD
	}, str);

	// check for input
	return relesGui(ctx, layId, rect);
}

void subWindowGui(guiContext* ctx, window* win) {
	ctx->child = win;
	ctx->inactive = 1;
	glfwMakeContextCurrent(ctx->win->gl); // hack for context
}

// pushes a non-interactive edit box
void editBoxGui(
	guiContext* ctx,
	guiLayerId layId,
	float4 rect,
	const char* str
) {
	// push quad
	quadGui(ctx, layId, rect, BG_DARK);
	borderGui(ctx, layId, rect, FG_DARK);

	// update displayed value and display
	textGui(ctx, layId, (float2){
		rect.x + 1 PAD, rect.y + 1 PAD
	}, str);
}

// pushes an interactive edit box
char* editorGui(
	guiContext* ctx,
	guiLayerId layId,
	float4 rect,
	const char* str,
	uint64_t id
) {
	// check for input
	int active, submit;
	char* in = bufferGui(ctx, layId, id, rect, &submit, &active);

	// make displayed value
	char disp[IN_BUF_SIZ];
	strncpy(disp, active ? in : str, IN_BUF_SIZ);
	disp[IN_BUF_SIZ - 1] = '\0';

	// blink curs
	double t = glfwGetTime();
	int blink = ((int)(t * 2.0)) & 1;
	if(active && blink) {
		int len = strlen(disp);
		if(len < IN_BUF_SIZ) disp[len] = '_';
	}

	// make gui
	editBoxGui(ctx, layId, rect, disp);

	// update actual value on submit
	if(submit) return in;
	return NULL;
}

int intGui(guiContext* ctx, guiLayerId layId, float4 rect, void* val) {
	// int to string
	char str[FIELD_SIZ];
	snprintf(str, FIELD_SIZ,"%d", *(int*)val);

	// update actual value on submit
	char* in = editorGui(ctx, layId, rect, str, (uint64_t)val + 1);
	if(in && val) {
		*(int*)val = atoi(in);
		return 1;
	}

	return 0;
}

int floatGui(guiContext* ctx, guiLayerId layId, float4 rect, void* val) {
	// int to string
	char str[FIELD_SIZ];
	snprintf(str, FIELD_SIZ, "%g", *(float*)val);

	// update actual value on submit
	char* in = editorGui(ctx, layId, rect, str, (uint64_t)val + 1);
	if(in && val) {
		*(float*)val = atof(in);
		return 1;
	}

	return 0;
}

int stringGui(guiContext* ctx, guiLayerId layId, float4 rect, void* val) {
	// temp. buffer 
	char str[FIELD_SIZ];
	snprintf(str, FIELD_SIZ, "%s", (char*)val);

	// update actual value on submit
	char* in = editorGui(ctx, layId, rect, str, (uint64_t)val + 1);
	if(in && val) {
		strncpy(val, in, IN_BUF_SIZ);
		return 1;
	}

	return 0;
}

// pushes a vector edit box 
int vectorGui(
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
	int ret = 0;
	for(int i = 0; i < n; i++) {
		if(floatGui(ctx, layId, rect, val + i * sizeof(float))) ret = 1; 
		rect.x += span + 1 PAD;
	}

	return ret;
}

// macro for vector edit boxes
#define VEC_FIELD_GUI(n)                                                           \
	int float##n##Gui(guiContext* ctx, guiLayerId layId, float4 rect, void* val) { \
	    return vectorGui(ctx, layId, rect, val, n);                                \
	}

// 2D vector edit box
VEC_FIELD_GUI(2)

// 3D vector edit box
VEC_FIELD_GUI(3)

// 4D vector edit box
VEC_FIELD_GUI(4)

// pushes a matrix row edit box 
int matrixRowGui(
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
	int ret = 0;
	for(int i = 0; i < n; i++) {
		if(floatGui(ctx, layId, rect, val + n * i * sizeof(float))) ret = 1;
		rect.x += span + 1 PAD;
	}

	return ret;
}

// macro for matrix edit boxes
#define MAT_FIELD_GUI(n)                                                         \
	int mat##n##Gui(guiContext* ctx, guiLayerId layId, float4 rect, void* val) { \
		int ret = 0;                                                             \
	    for(int i = 0; i < n; i++) {                                             \
	        if(matrixRowGui(ctx, layId, rect, ((float*)val) + i, n)) ret = 1;    \
	        downGui(ctx, layId, rect.w + 1 PAD);                                 \
	    }                                                                        \
	                                                                             \
	    return ret;                                                              \
	}                                                                            \

// 2x2 matrix edit box
MAT_FIELD_GUI(2)

// 3x3 matrix edit box
MAT_FIELD_GUI(3)

// 4x4 matrix edit box
MAT_FIELD_GUI(4)

// GUI for a data reference
int dataGui(
	guiContext* ctx,
	guiLayerId layId,
	float4 rect,
	dataRef** ref,
	dataTable* tab
) {
	// ref to string 
	char str[FIELD_SIZ];
	snprintf(str, FIELD_SIZ, "%.*s", FIELD_SIZ - 1, 
		*ref ? (*ref)->path : "(null path)");

	// make gui
	rect.z -= 3 PAD + ICO_SIZ;
	editBoxGui(ctx, layId, rect, str);

	// search button
	if(buttonGui(ctx, SCROLL, (float4){
		rect.x + rect.z + 1 PAD, rect.y,
		2 PAD + ICO_SIZ, 1 PAD + ICO_SIZ 
	}, ICO_SEARCH, "")) {
		// set data pointer in context
		ctx->in.dataPtr = ref;

		subWindowGui(ctx, newWindow(
			DATASEL_WIDTH,
			DATASEL_HEIGHT,
			"Select Data",
			makeDataselCallback(ref, tab, ctx)
		));
	}

	// return if changed
	if(ctx->in.dataPtr == ref && ctx->in.dataSet) {
		ctx->in.dataSet = 0;
		return 1;
	}

	return 0;
}

int textureGui(guiContext* ctx, guiLayerId layId, float4 rect, void* val) {
	return dataGui(ctx, layId, rect, val, &textureTable);
}

int meshGui(guiContext* ctx, guiLayerId layId, float4 rect, void* val) {
	return dataGui(ctx, layId, rect, val, &meshTable);
}

int materialGui(guiContext* ctx, guiLayerId layId, float4 rect, void* val) {
	return dataGui(ctx, layId, rect, val, &materialTable);
}
