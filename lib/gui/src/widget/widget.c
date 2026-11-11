#include "widget.h"
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -- input primitives

int guiHover(guiContext* ctx, guiLayerId layId, guiRect rect) {
	rect.y += ctx->layers[layId].vPos;
	return ctx->in.xCur >= rect.x && ctx->in.xCur <= rect.x + rect.w &&
	       ctx->in.yCur >= rect.y && ctx->in.yCur <= rect.y + rect.h;
}

int guiRelease(guiContext* ctx, guiLayerId layId, guiRect rect) {
	int reles = guiHover(ctx, layId, rect) && ctx->in.curReles;

	// consume other input
	if(reles
	|| (guiHover(ctx, layId, rect) && ctx->in.curPress)) {
		ctx->in.curReles = ctx->in.curPress = ctx->in.curDown = 0;
	}
	
	return reles;
}

int guiPress(guiContext* ctx, guiLayerId layId, guiRect rect) {
	int press = guiHover(ctx, layId, rect) && ctx->in.curPress;

	// consume other input
	if(press
	|| (guiHover(ctx, layId, rect) && ctx->in.curReles)) {
		ctx->in.curReles = ctx->in.curPress = ctx->in.curDown = 0;
	}

	return press;
}

char* guiBuffer(
	guiContext* ctx,
	guiLayerId layId,
	uint64_t id,
	guiRect rect,
	int* submit,
	int* active
) {
	*active = *submit = 0;

	// check if pressed
	int press = guiPress(ctx, layId, rect);

	// activate on press
	if((ctx->in.hotId == 0) && press) {
		// set id
		ctx->in.hotId = id;

		// clear buffer
		memset(ctx->in.keyBuf, 0, GUI_IN_BUF_SIZ);
		ctx->in.keyBufSiz = 0;

		return ctx->in.keyBuf;
	}

	// check if active
	if(ctx->in.hotId != id) return NULL;
	*active = 1;

	// discard on outside press or escape
	if((!guiHover(ctx, layId, rect) && ctx->in.curDown) || ctx->in.escape) {
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

void guiScroll(guiContext* ctx, guiLayerId layId) {
	float max = 0.0f;
	float min = -ctx->layers[layId].lastHeight
	          + WIN_HEIGHT 
	          + ctx->layers[FIXED].lastHeight;

	// get absolute scroll
	ctx->in.absScroll += ctx->in.scroll * GUI_SCROLL_SENSITIVITY;
	if(ctx->in.absScroll < min) ctx->in.absScroll = min;
	if(ctx->in.absScroll > max) ctx->in.absScroll = max;

	// scroll vPos
	ctx->layers[layId].vPos += ctx->in.absScroll;

	// consume scroll
	ctx->in.scroll = 0.0f;
}

// -- rendering primitives

void guiDown(guiContext* ctx, guiLayerId layId, float amt) {
	ctx->layers[layId].vPos += amt;
	ctx->layers[layId].height += amt;
}

void guiTrim(guiContext* ctx, guiLayerId layId) {
	window* win = ctx->win;
	if(ctx->layers[layId].height != ctx->layers[layId].lastHeight)
		winResize(win, win->width, ctx->layers[layId].vPos);
}

void guiPushQuad(guiContext* ctx, guiLayerId layId, guiRect rect, guiRect uv) {
	guiPush(&ctx->layers[layId], (guiQuad){
		rect.x, rect.y + ctx->layers[layId].vPos, 
		rect.w, rect.h,
		uv
	});
}

void guiBorder(
	guiContext* ctx,
	guiLayerId layId,
	guiRect rect,
	guiRect uv
) {
	// i hate high DPI displays
	float border = fbToWinH(ctx->win, GUI_BORDER);

	// push border quads
	guiPushQuad(ctx, layId, (guiRect){rect.x,                   rect.y,                   rect.w, border}, uv);
	guiPushQuad(ctx, layId, (guiRect){rect.x,                   rect.y + rect.h - border, rect.w, border}, uv);
	guiPushQuad(ctx, layId, (guiRect){rect.x,                   rect.y,                   border, rect.h}, uv);
	guiPushQuad(ctx, layId, (guiRect){rect.x + rect.w - border, rect.y,                   border, rect.h}, uv);
}

void guiTextN(
	guiContext* ctx,
	guiLayerId layId,
	guiPos pos,
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
		guiRect uv = {
			GUI_UV((float)tX / 2, tY),
			GUI_UV((float)tX / 2 + 0.5f, tY + 1.0f)
		};

		// push characters
		guiPush(&ctx->layers[layId], (guiQuad){
			pos.x + n * GUI_TXT_WIDTH, ctx->layers[layId].vPos + pos.y, 
			GUI_TXT_WIDTH, GUI_TXT_HEIGHT,
			uv
		});

		// move right
		n++;
		if(n == len) break;
	}
}

void guiText(
	guiContext* ctx,
	guiLayerId layId,
	guiPos pos,
	const char* str
) {
	guiTextN(ctx, layId, pos, str, strlen(str));
}

void guiSeparator(
	guiContext* ctx,
	guiLayerId layId,
	guiRect rect,
	const char* str
) {
	float txtWidth = GUI_TXT_WIDTH * (strlen(str) + 1);

	// background line
	guiPushQuad(ctx, layId, (guiRect){
		rect.x + txtWidth, rect.y + rect.h / 2,
		rect.w - txtWidth, GUI_BORDER / 2
	}, GUI_FG_DARK);

	// text
	guiText(ctx, layId, (guiPos){
		rect.x, rect.y
	}, str);
}

void guiIcon(guiContext* ctx, guiLayerId layId, guiPos pos, guiRect uv) {
	guiPush(&ctx->layers[layId], (guiQuad){
		pos.x - GUI_ICO_OFFSET / 2.0f, 
		pos.y + ctx->layers[layId].vPos - GUI_ICO_OFFSET / 2.0f,
		GUI_ICO_SIZ + GUI_ICO_OFFSET, GUI_ICO_SIZ + GUI_ICO_OFFSET,
		uv
	});
}

int guiButton(
	guiContext* ctx,
	guiLayerId layId,
	guiRect rect,
	guiRect ico,
	const char* str
) {
	int press = guiHover(ctx, layId, rect)
	            && ctx->in.curDown;

	// push quad
	guiPushQuad(ctx, layId, rect, GUI_BG_LIGHT);
	guiBorder(ctx, layId, rect, GUI_FG_DARK);

	// display icon
	guiIcon(ctx, layId, (guiPos){
		rect.x + 1 GUI_PAD,
		rect.y + (press ? 1 GUI_PAD + GUI_HPAD: 1 GUI_PAD) // raise on click
	}, ico);

	// display text
	guiText(ctx, layId, (guiPos){
		rect.x + 2 GUI_PAD + GUI_ICO_SIZ, rect.y + 1 GUI_PAD
	}, str);

	// check for input
	return guiRelease(ctx, layId, rect);
}

void guiChildWindow(guiContext* ctx, window* win) {
	ctx->child = win;
	ctx->inactive = 1;
	glfwMakeContextCurrent(ctx->win->gl); // hack for context
}

void guiTextBox(
	guiContext* ctx,
	guiLayerId layId,
	guiRect rect,
	const char* str
) {
	// push quad
	guiPushQuad(ctx, layId, rect, GUI_BG_DARK);
	guiBorder(ctx, layId, rect, GUI_FG_DARK);

	// get size of output buffer
	int outSiz = (int)(rect.w / GUI_TXT_WIDTH) - 1;

	// update displayed value and display
	guiTextN(ctx, layId, (guiPos){
		rect.x + 1 GUI_PAD, rect.y + 1 GUI_PAD
	}, str, outSiz);
}

char* guiEditBox(
	guiContext* ctx,
	guiLayerId layId,
	guiRect rect,
	const char* str,
	uint64_t id
) {
	// check for input
	int active, submit;
	char* in = guiBuffer(ctx, layId, id, rect, &submit, &active);

	// make displayed value
	char disp[GUI_IN_BUF_SIZ];
	strncpy(disp, active ? in : str, GUI_IN_BUF_SIZ);
	disp[GUI_IN_BUF_SIZ - 1] = '\0';

	// blink curs
	double t = glfwGetTime();
	int blink = ((int)(t * 2.0)) & 1;
	if(active && blink) {
		int len = strlen(disp);
		if(len < GUI_IN_BUF_SIZ) disp[len] = '_';
	}

	// make gui
	guiTextBox(ctx, layId, rect, disp);

	// update actual value on submit
	if(submit) return in;
	return NULL;
}

// -- example widgets

int intGui(guiContext* ctx, guiLayerId layId, guiRect rect, void* val) {
	// int to string
	char str[GUI_OUT_BUF_SIZ];
	snprintf(str, GUI_OUT_BUF_SIZ, "%d", *(int*)val);

	// update actual value on submit
	char* in = guiEditBox(ctx, layId, rect, str, (uint64_t)val + 1);
	if(in && val) {
		*(int*)val = atoi(in);
		return 1;
	}

	return 0;
}

int floatGui(guiContext* ctx, guiLayerId layId, guiRect rect, void* val) {
	// int to string
	char str[GUI_OUT_BUF_SIZ];
	snprintf(str, GUI_OUT_BUF_SIZ, "%g", *(float*)val);

	// update actual value on submit
	char* in = guiEditBox(ctx, layId, rect, str, (uint64_t)val + 1);
	if(in && val) {
		*(float*)val = atof(in);
		return 1;
	}

	return 0;
}

int stringGui(guiContext* ctx, guiLayerId layId, guiRect rect, void* val) {
	// temp. buffer 
	char str[GUI_OUT_BUF_SIZ];
	snprintf(str, GUI_OUT_BUF_SIZ, "%s", (char*)val);

	// update actual value on submit
	char* in = guiEditBox(ctx, layId, rect, str, (uint64_t)val + 1);
	if(in && val) {
		strncpy(val, in, GUI_IN_BUF_SIZ);
		((char*)val)[GUI_IN_BUF_SIZ - 1] = '\0';
		return 1;
	}

	return 0;
}
