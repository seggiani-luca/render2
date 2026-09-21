#include "widget.h"
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// -- input primitives

int hoverGui(guiContext* ctx, guiLayerId layId, rectangle rect) {
	rect.y += ctx->layers[layId].vPos;
	return ctx->in.xCur >= rect.x && ctx->in.xCur <= rect.x + rect.w &&
	       ctx->in.yCur >= rect.y && ctx->in.yCur <= rect.y + rect.h;
}

int relesGui(guiContext* ctx, guiLayerId layId, rectangle rect) {
	int reles = hoverGui(ctx, layId, rect) && ctx->in.curReles;

	// consume other input
	if(reles
	|| (hoverGui(ctx, layId, rect) && ctx->in.curPress)) {
		ctx->in.curReles = ctx->in.curPress = ctx->in.curDown = 0;
	}
	
	return reles;
}

int pressGui(guiContext* ctx, guiLayerId layId, rectangle rect) {
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
	rectangle rect,
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
	if(ctx->layers[SCROLL].height != ctx->layers[SCROLL].lastHeight)
		resizeWindow(win, win->width, ctx->layers[SCROLL].vPos);
}

void quadGui(guiContext* ctx, guiLayerId layId, rectangle rect, rectangle uv) {
	pushGui(&ctx->layers[layId], (quad){
		rect.x, rect.y + ctx->layers[layId].vPos, 
		rect.w, rect.h,
		uv
	});
}

void borderGui(
	guiContext* ctx,
	guiLayerId layId,
	rectangle rect,
	rectangle uv
) {
	// i hate high DPI displays
	float border = fbToWinH(ctx->win, BORDER);

	quadGui(ctx, layId, (rectangle){rect.x,                   rect.y,                   rect.w, border}, uv);
	quadGui(ctx, layId, (rectangle){rect.x,                   rect.y + rect.h - border, rect.w, border}, uv);
	quadGui(ctx, layId, (rectangle){rect.x,                   rect.y,                   border, rect.h}, uv);
	quadGui(ctx, layId, (rectangle){rect.x + rect.w - border, rect.y,                   border, rect.h}, uv);
}

void textGuiN(
	guiContext* ctx,
	guiLayerId layId,
	position pos,
	const char* str,
	size_t len
) {
	char c;
	size_t n = 0;

	// go through all characters
	while((c = *str++)) {
		// extract character index
		int tX = (c - 32) % 32;
		int tY = (c - 32) / 32;

		// calculate UV
		rectangle uv = {
			UV((float)tX / 2, tY),
			UV((float)tX / 2 + 0.5f, tY + 1.0f)
		};

		// push characters
		pushGui(&ctx->layers[layId], (quad){
			pos.x + n * TXT_WIDTH, ctx->layers[layId].vPos + pos.y, 
			TXT_WIDTH, TXT_HEIGHT,
			uv
		});

		// move right
		n++;
		if(n == len) break;
	}
}

void textGui(guiContext* ctx, guiLayerId layId, position pos, const char* str) {
	textGuiN(ctx, layId, pos, str, strlen(str));
}

void separatorGui(guiContext* ctx, guiLayerId layId, rectangle rect, const char* str) {
	float txtWidth = TXT_WIDTH * (strlen(str) + 1);

	// background line
	quadGui(ctx, layId, (rectangle){
		rect.x + txtWidth, rect.y + rect.h / 2,
		rect.w - txtWidth, BORDER / 2
	}, FG_DARK);

	// text
	textGui(ctx, layId, (position){
		rect.x, rect.y
	}, str);
}

void iconGui(guiContext* ctx, guiLayerId layId, position pos, rectangle uv) {
	pushGui(&ctx->layers[layId], (quad){
		pos.x - ICO_OFFSET / 2.0f, 
		pos.y + ctx->layers[layId].vPos - ICO_OFFSET / 2.0f,
		ICO_SIZ + ICO_OFFSET, ICO_SIZ + ICO_OFFSET,
		uv
	});
}

int buttonGui(
	guiContext* ctx,
	guiLayerId layId,
	rectangle rect,
	rectangle ico,
	const char* str
) {
	int press = hoverGui(ctx, layId, rect)
	            && ctx->in.curDown;

	// push quad
	quadGui(ctx, layId, rect, BG_LIGHT);
	borderGui(ctx, layId, rect, FG_DARK);

	// display icon
	iconGui(ctx, layId, (position){
		rect.x + 1 PAD, rect.y + (press ? 1 PAD + HPAD: 1 PAD)
	}, ico);

	// display text
	textGui(ctx, layId, (position){
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

void textBoxGui(
	guiContext* ctx,
	guiLayerId layId,
	rectangle rect,
	const char* str
) {
	// push quad
	quadGui(ctx, layId, rect, BG_DARK);
	borderGui(ctx, layId, rect, FG_DARK);

	// get size of output buffer
	int outSiz = (int)(rect.w / TXT_WIDTH) - 1;

	// update displayed value and display
	textGuiN(ctx, layId, (position){
		rect.x + 1 PAD, rect.y + 1 PAD
	}, str, outSiz);
}

char* editBoxGui(
	guiContext* ctx,
	guiLayerId layId,
	rectangle rect,
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
	textBoxGui(ctx, layId, rect, disp);

	// update actual value on submit
	if(submit) return in;
	return NULL;
}

// -- example widgets

int intGui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val) {
	// int to string
	char str[OUT_BUF_SIZ];
	snprintf(str, OUT_BUF_SIZ,"%d", *(int*)val);

	// update actual value on submit
	char* in = editBoxGui(ctx, layId, rect, str, (uint64_t)val + 1);
	if(in && val) {
		*(int*)val = atoi(in);
		return 1;
	}

	return 0;
}

int floatGui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val) {
	// int to string
	char str[OUT_BUF_SIZ];
	snprintf(str, OUT_BUF_SIZ, "%g", *(float*)val);

	// update actual value on submit
	char* in = editBoxGui(ctx, layId, rect, str, (uint64_t)val + 1);
	if(in && val) {
		*(float*)val = atof(in);
		return 1;
	}

	return 0;
}

int stringGui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val) {
	// temp. buffer 
	char str[OUT_BUF_SIZ];
	snprintf(str, OUT_BUF_SIZ, "%s", (char*)val);

	// update actual value on submit
	char* in = editBoxGui(ctx, layId, rect, str, (uint64_t)val + 1);
	if(in && val) {
		strncpy(val, in, IN_BUF_SIZ);
		((char*)val)[IN_BUF_SIZ - 1] = '\0';
		return 1;
	}

	return 0;
}
