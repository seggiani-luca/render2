#include "widget.h"
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// -- input primitives

int hoverGui(guiContext* ctx, float4 rect) {
	rect.y += ctx->vPos;
	return ctx->in.xCur >= rect.x && ctx->in.xCur <= rect.x + rect.z
	    && ctx->in.yCur >= rect.y && ctx->in.yCur <= rect.y + rect.w;
}

char* bufferGui(
	guiContext* ctx,
	uint64_t id,
	float4 rect,
	int* submit,
	int* active
) {
	*active = *submit = 0;

	// activate on press
	if((ctx->in.hotId == 0) && hoverGui(ctx, rect) && ctx->in.curPress) {
		ctx->in.hotId = id;
		memset(ctx->in.keyBuf, 0, IN_BUF_SIZ);
		ctx->in.keyBufSiz = 0;
		return NULL;
	}

	// check if active
	if(ctx->in.hotId != id) return NULL;
	*active = 1;

	// discard on any press
	if(!hoverGui(ctx, rect) && ctx->in.curPress) {
		*active = 0;
		ctx->in.hotId = 0;
		return NULL;
	}

	// return on complete
	if(ctx->in.enter) {
		ctx->in.keyBuf[ctx->in.keyBufSiz] = '\0';
		ctx->in.hotId = 0;
		
		*submit = 1;
		return ctx->in.keyBuf;
	}

	return ctx->in.keyBuf;
}

// -- rendering primitives

void quadGui(guiContext* ctx, float4 rect, float4 uv) {
	pushGui(ctx, (quad) {
		{ rect.x, rect.y + ctx->vPos, rect.z, rect.w },
		uv
	});
}

#define W 0.5f // width
void borderGui(guiContext* ctx, float4 rect, float4 uv) {
	quadGui(ctx, (float4){ rect.x, rect.y,              rect.z, W }, uv);
	quadGui(ctx, (float4){ rect.x, rect.y + rect.w - W, rect.z, W }, uv);
	quadGui(ctx, (float4){ rect.x, rect.y,              W,      rect.w }, uv);
	quadGui(ctx, (float4){ rect.x + rect.z - W, rect.y, W,      rect.w }, uv);
}

void textGui(guiContext* ctx, float2 pos, const char* str) {
	char c;
	int w = 0;
	while((c = *str++)) {
		int tX = (c - 32) % 32;
		int tY = (c - 32) / 32;

		pushGui(ctx, (quad) {
			{pos.x + w, ctx->vPos + pos.y, TXT_WIDTH, TXT_HEIGHT},
			{UV((float)tX / 2, tY), UV((float)tX / 2 + 0.5f, tY + 1.0f)}
		});

		w += TXT_WIDTH; 
	}
}

void iconGui(guiContext* ctx, float2 pos, float4 uv) {
	pushGui(ctx, (quad) {
		{pos.x, pos.y + ctx->vPos, ICO_SIZ, ICO_SIZ},
		uv
	});
}

int buttonGui(guiContext* ctx, float4 rect, float4 ico, const char* str) {
	// push quad
	quadGui(ctx, rect, BG_LIGHT);
	borderGui(ctx, rect, FG_DARK);

	// display icon + text 
	iconGui(ctx, (float2) {rect.x + PAD, rect.y + PAD}, ico);
	textGui(ctx, (float2) {rect.x + ICO_SIZ + PAD * 2, rect.y + PAD}, str);

	// check for input
	return hoverGui(ctx, rect) && ctx->in.curReles;
}

void subWindowGui(
	guiContext* ctx,
	int width,
	int height,
	const char* title,
	renderCallback cback,
	float4 rect,
	float4 ico,
	const char* str
) {
	// push quad
	quadGui(ctx, rect, BG_LIGHT);
	borderGui(ctx, rect, FG_DARK);

	// display icon + text 
	iconGui(ctx, (float2) {rect.x + PAD, rect.y + PAD}, ico);
	textGui(ctx, (float2) {rect.x + ICO_SIZ + PAD * 2, rect.y + PAD}, str);
	
	if(ctx->child) {
		if(!updateWindow(ctx->child)) {
			freeWindow(ctx->child);
			ctx->child = NULL;
			ctx->inactive = 0;
		}
		glfwMakeContextCurrent(ctx->win->gl); // hack for context

		return;
	}

	// check for input
	if(hoverGui(ctx, rect) && ctx->in.curReles) {
		ctx->child = newWindow(
			width,
			height,
			title,
			cback
		);
		glfwMakeContextCurrent(ctx->win->gl); // hack for context
		
		ctx->inactive = 1;
	}
}

void intGui(guiContext* ctx, float4 rect, int* val) {
	// int to string
	char str[FIELD_SIZ];
	snprintf(str, FIELD_SIZ, "%d", *val);

	// check for input
	int active, submit; 
	char* in = bufferGui(ctx, (uint64_t) val, rect, &submit, &active);

	// push quad
	quadGui(ctx, rect, BG_DARK);
	borderGui(ctx, rect, FG_DARK);
	
	// update displayed value and display
	textGui(ctx, (float2) {rect.x + PAD, rect.y + PAD}, active ? in : str);

	// update actual value on submit
	if(submit) *val = atoi(in);
}

void floatGui(guiContext* ctx, float4 rect, float* val) {
	// int to string
	char str[FIELD_SIZ];
	snprintf(str, FIELD_SIZ, "%g", *val);

	// check for input
	int active, submit; 
	char* in = bufferGui(ctx, (uint64_t) val, rect, &submit, &active);

	// push quad
	quadGui(ctx, rect, BG_DARK);
	borderGui(ctx, rect, FG_DARK);
	
	// update displayed value and display
	textGui(ctx, (float2) {rect.x + PAD, rect.y + PAD}, active ? in : str);

	// update actual value on submit
	if(submit) *val = atof(in);
}

void stringGui(guiContext* ctx, float4 rect, char* val) {
	// int to string
	char str[FIELD_SIZ];
	snprintf(str, FIELD_SIZ, "%s", val);

	// check for input
	int active, submit; 
	char* in = bufferGui(ctx, (uint64_t) val, rect, &submit, &active);

	// push quad
	quadGui(ctx, rect, BG_DARK);
	borderGui(ctx, rect, FG_DARK);
	
	// update displayed value and display
	textGui(ctx, (float2) {rect.x + PAD, rect.y + PAD}, active ? in : str);

	// update actual value on submit
	if(submit) {
		strncpy(val, in, IN_BUF_SIZ);
	}
}
