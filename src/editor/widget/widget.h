#ifndef EDITOR_WIDGET_H
#define EDITOR_WIDGET_H

#include "../../gui/gui.h"
#include "../../gui/widget/widget.h"

// -- constants

// offset for named boxes 
#define NAME_OFF 84.0f

// -- widgets

// pushes a 2D vector edit box
int float2Gui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val);

// pushes a 3D vector edit box
int float3Gui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val);

// pushes a 4D vector edit box
int float4Gui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val);

// pushes a 2x2 matrix edit box
int mat2Gui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val);

// pushes a 3x3 matrix edit box
int mat3Gui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val);

// pushes a 4x4 matrix edit box
int mat4Gui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val);

// pushes a transform edit box
int transformGui(guiContext* ctx, 
	guiLayerId layId,
	rectangle rect,
	void* val
);

// pushes a camera edit box
int cameraGui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val);

// pushes an atmosphere edit box
int atmosphereGui(
	guiContext* ctx,
	guiLayerId layId,
	rectangle rect,
	void* val
);

#endif
