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

float quadGui(guiContext* ctx, float4 rect, float4 uv) {
	pushGui(ctx, (quad) {
		{ rect.x, rect.y + ctx->vPos, rect.z, rect.w },
		uv
	});

	return rect.y;
}

float textGui(guiContext* ctx, float2 rPos, const char* str) {
	char c;
	int w = 0;
	while((c = *str++)) {
		int tX = (c - 32) % 32;
		int tY = (c - 32) / 32;

		pushGui(ctx, (quad) {
			{rPos.x + w, ctx->vPos + rPos.y, TXT_WIDTH, TXT_HEIGHT},
			{UV((float)tX / 2, tY), UV((float)tX / 2 + 0.5f, tY + 1.0f)}
		});

		w += TXT_WIDTH; 
	}

	return w;
}

void iconGui(guiContext* ctx, float2 rPos, float4 uv) {
	pushGui(ctx, (quad) {
		{rPos.x, rPos.y + ctx->vPos, ICO_SIZ, ICO_SIZ},
		uv
	});
}

float intGui(guiContext* ctx, float4 rect, int* val) {
	// int to string
	char str[FIELD_SIZ];
	snprintf(str, FIELD_SIZ, "%d", *val);

	// check for input
	int active, submit; 
	char* in = bufferGui(ctx, (uint64_t) val, rect, &submit, &active);

	// push quad
	quadGui(ctx, rect, BG_DARK);
	
	// update displayed value and display
	textGui(ctx, (float2) {rect.x + PAD, rect.y + PAD}, active ? in : str);

	// update actual value on submit
	if(submit) *val = atoi(in);

	return rect.y;
}

float floatGui(guiContext* ctx, float4 rect, float* val) {
	// int to string
	char str[FIELD_SIZ];
	snprintf(str, FIELD_SIZ, "%g", *val);

	// check for input
	int active, submit; 
	char* in = bufferGui(ctx, (uint64_t) val, rect, &submit, &active);

	// push quad
	quadGui(ctx, rect, BG_DARK);
	
	// update displayed value and display
	textGui(ctx, (float2) {rect.x + PAD, rect.y + PAD}, active ? in : str);

	// update actual value on submit
	if(submit) *val = atof(in);

	return rect.y;
}

float stringGui(guiContext* ctx, float4 rect, char* val) {
	// int to string
	char str[FIELD_SIZ];
	snprintf(str, FIELD_SIZ, "%s", val);

	// check for input
	int active, submit; 
	char* in = bufferGui(ctx, (uint64_t) val, rect, &submit, &active);

	// push quad
	quadGui(ctx, rect, BG_DARK);
	
	// update displayed value and display
	textGui(ctx, (float2) {rect.x + PAD, rect.y + PAD}, active ? in : str);

	// update actual value on submit
	if(submit) {
		strncpy(val, in, IN_BUF_SIZ);
	} 

	return rect.y;
}
