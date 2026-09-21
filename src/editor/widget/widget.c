#include "widget.h"

#include "../selector/selector.h"
#include "../../math/math.h"
#include "../../render/render.h"

// pushes a vector edit box 
int vectorGui(
	guiContext* ctx,
	guiLayerId layId,
	rectangle rect,
	void* val,
	int n
) {
	// calculate float edit box span
	float span = (rect.w - (n - 1) * 1 PAD) / n;
	rect.w = span;

	// make float edit boxes
	int ret = 0;
	for(int i = 0; i < n; i++) {
		if(floatGui(ctx, layId, rect, val + i * sizeof(float))) ret = 1; 
		rect.x += span + 1 PAD;
	}

	return ret;
}

// macro for vector edit boxes
#define VEC_FIELD_GUI(n)                                                              \
	int float##n##Gui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val) { \
	    return vectorGui(ctx, layId, rect, val, n);                                   \
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
	rectangle rect,
	void* val,
	int n
) {
	// calculate float edit box span
	float span = (rect.w - (n - 1) * 1 PAD) / n;
	rect.w = span;

	// make float edit boxes
	int ret = 0;
	for(int i = 0; i < n; i++) {
		if(floatGui(ctx, layId, rect, val + n * i * sizeof(float))) ret = 1;
		rect.x += span + 1 PAD;
	}

	return ret;
}

// macro for matrix edit boxes
#define MAT_FIELD_GUI(n)                                                            \
	int mat##n##Gui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val) { \
	    int ret = 0;                                                                \
	    for(int i = 0; i < n; i++) {                                                \
	        if(matrixRowGui(ctx, layId, rect, ((float*)val) + i, n)) ret = 1;       \
	        downGui(ctx, layId, rect.h + 1 PAD);                                    \
	    }                                                                           \
	                                                                                \
	    return ret;                                                                 \
	}                                                                               \

// 2x2 matrix edit box
MAT_FIELD_GUI(2)

// 3x3 matrix edit box
MAT_FIELD_GUI(3)

// 4x4 matrix edit box
MAT_FIELD_GUI(4)

int transformGui(guiContext* ctx, 
	guiLayerId layId,
	rectangle rect,
	void* val
) {
	rect.x += NAME_OFF;
	rect.w -= NAME_OFF;

	// modify return
	int ret = 0;

	// position
	if(float3Gui(ctx, layId, rect, ((float3*)val))) ret = 1;
	downGui(ctx, layId, rect.h + 1 PAD);
	textGui(ctx, SCROLL, (position){
		2 PAD, 3 PAD
	}, "Position");

	// rotation
	if(float3Gui(ctx, layId, rect, ((float3*)val) + 1)) ret = 1;
	downGui(ctx, layId, rect.h + 1 PAD);
	textGui(ctx, SCROLL, (position){
		2 PAD, 3 PAD 
	}, "Rotation");

	// scale
	if(float3Gui(ctx, layId, rect, ((float3*)val) + 2)) ret = 1;
	downGui(ctx, layId, rect.h + 1 PAD);
	textGui(ctx, SCROLL, (position){
		2 PAD, 3 PAD 
	}, "Scale");

	return ret;
}

int cameraGui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val) {
	rect.x += NAME_OFF;
	rect.w -= NAME_OFF;

	camera* c = (camera*)val;

	// modify return
	int ret = 0;

	// fov
	if(floatGui(ctx, layId, rect, &c->fov)) ret = 1;
	downGui(ctx, layId, rect.h + 1 PAD);
	textGui(ctx, SCROLL, (position){
		2 PAD, 3 PAD
	}, "Fov");

	// near
	if(floatGui(ctx, layId, rect, &c->nearPlane)) ret = 1;
	downGui(ctx, layId, rect.h + 1 PAD);
	textGui(ctx, SCROLL, (position){
		2 PAD, 3 PAD 
	}, "Near");

	// far
	if(floatGui(ctx, layId, rect, &c->farPlane)) ret = 1;
	downGui(ctx, layId, rect.h + 1 PAD);
	textGui(ctx, SCROLL, (position){
		2 PAD, 3 PAD 
	}, "Far");

	return ret;
}

// pushes an atmosphere edit box
int atmosphereGui(
	guiContext* ctx,
	guiLayerId layId,
	rectangle rect,
	void* val
) {
	rect.x += NAME_OFF;
	rect.w -= NAME_OFF;

	atmosphere* a = (atmosphere*)val;
	
	// modify return
	int ret = 0;

	// sun 
	if(float3Gui(ctx, layId, rect, &a->sun)) ret = 1;
	downGui(ctx, layId, rect.h + 1 PAD);
	textGui(ctx, SCROLL, (position){
		2 PAD, 3 PAD
	}, "Sun Color");

	// ambient
	downGui(ctx, SCROLL, TXT_HEIGHT + 2 PAD);
	separatorGui(ctx, SCROLL, (rectangle){
		2 PAD, 3 PAD,
		WIN - 4 PAD, TXT_HEIGHT
	}, "Ambient");

	if(float3Gui(ctx, layId, rect, &a->ambient)) ret = 1;
	downGui(ctx, layId, rect.h + 1 PAD);
	textGui(ctx, SCROLL, (position){
		2 PAD, 3 PAD
	}, "Color");
	
	if(textureGui(ctx, layId, rect, &a->ambientMap)) ret = 1;
	downGui(ctx, layId, rect.h + 1 PAD);
	textGui(ctx, SCROLL, (position){
		2 PAD, 3 PAD
	}, "Cubemap");
		
	// background
	downGui(ctx, SCROLL, TXT_HEIGHT + 2 PAD);
	separatorGui(ctx, SCROLL, (rectangle){
		2 PAD, 3 PAD,
		WIN - 4 PAD, TXT_HEIGHT
	}, "Sky");
	
	if(float3Gui(ctx, layId, rect, &a->background)) ret = 1;
	downGui(ctx, layId, rect.h + 1 PAD);
	textGui(ctx, SCROLL, (position){
		2 PAD, 3 PAD
	}, "Color");
	
	if(textureGui(ctx, layId, rect, &a->backgroundMap)) ret = 1;
	downGui(ctx, layId, rect.h + 1 PAD);
	textGui(ctx, SCROLL, (position){
		2 PAD, 3 PAD
	}, "Cubemap");

	return ret;
}
