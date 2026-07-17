#ifndef GUI_WIDGET_H
#define GUI_WIDGET_H

#include "../../math/math.h"
#include "../gui.h"
#include "../render/render.h"

// -- constants

// size of field entry box
#define FIELD_SIZ 64

// sizing contants
#define PAD     * 4.0f
#define HPAD    (1 PAD / 2.0f)
#define ROW     48.0f
#define HROW    (ROW / 2.0f)
#define BORDER  2.0f

// window dependent sizing constants
#define WIN  (ctx->win->width)
#define HWIN (ctx->win->width / 2.0f)
#define HEIG (ctx->win->height)

// atlas sections
#define AU       (1.0f / 16)
#define UV(x, y) ((float)(x) * AU), (1.0f - (float)(y) * AU)

// atlas colors
#define BG_ABS   (float4) { UV(0, 3), UV(1, 4) }
#define BG_DARK  (float4) { UV(1, 3), UV(2, 4) }
#define BG_LIGHT (float4) { UV(2, 3), UV(3, 4) }
#define FG_DARK  (float4) { UV(3, 3), UV(4, 4) }
#define FG_LIGHT (float4) { UV(4, 3), UV(5, 4) }

// atlas text sizing
#define TXT_WIDTH  (0.5f * AU * ATLAS_SIZ)
#define TXT_HEIGHT (TXT_WIDTH * 2)

// atlas icons
#define ICO(x, y)  (float4) { UV(x, y), UV(x + 1,  y + 1) }
#define ICO_SIZ    TXT_HEIGHT
#define ICO_ENTITY ICO(0, 4)
#define ICO_INT    ICO(1, 4)
#define ICO_FLOAT  ICO(2, 4)
#define ICO_STRING ICO(3, 4)
#define ICO_SCENE  ICO(4, 4)
#define ICO_FLOAT2 ICO(5, 4)
#define ICO_FLOAT3 ICO(6, 4)
#define ICO_FLOAT4 ICO(7, 4)
#define ICO_MAT2   ICO(8, 4)
#define ICO_MAT3   ICO(9, 4)
#define ICO_MAT4   ICO(10, 4)
#define ICO_QUAT   ICO(11, 4)
#define ICO_TRANS  ICO(12, 4)
#define ICO_POS    ICO(13, 4)
#define ICO_ROT    ICO(14, 4)
#define ICO_SCALE  ICO(15, 4)

#define ICO_NEW    ICO(0, 5)
#define ICO_DELETE ICO(1, 5)
#define ICO_MOVE   ICO(2, 5)
#define ICO_INDENT ICO(3, 5)
#define ICO_TEX    ICO(4, 5)
#define ICO_MESH   ICO(5, 5)
#define ICO_MAT    ICO(6, 5)
#define ICO_SEARCH ICO(7, 5)
#define ICO_FILE   ICO(8, 5)
#define ICO_NUFILE ICO(9, 5)

// -- sizing

// data selector size
#define DATASEL_WIDTH 600
#define DATASEL_HEIGHT 400

// offset for import button
#define IMPORT_OFF 520.0f

// offset for path button
#define PATH_OFF 50.0f 

// offset for transform fields
#define TRANS_OFF 84.0f

// -- input primitives

// test for mouse hover on rect
int hoverGui(guiContext* ctx, guiLayerId layId, float4 rect);

// test for mouse release on rect
int relesGui(guiContext* ctx, guiLayerId layId, float4 rect);

// test for mouse press on rect
int pressGui(guiContext* ctx, guiLayerId layId, float4 rect);

// reads text input
char* bufferGui(
	guiContext* ctx,
	guiLayerId layId,
	uint64_t id,
	float4 rect,
	int* submit,
	int* active
);

// scroll sensitivity
#define SCROLL_SENS 4.0f

// scrolls GUI
void scrollGui(guiContext* ctx, guiLayerId layId);

// -- rendering primitives

// adds vertical space to GUI
void downGui(guiContext* ctx, guiLayerId layId, float amt);

// trims GUI to vertical space
void trimGui(guiContext* ctx);

// pushes a quad
void quadGui(guiContext* ctx, guiLayerId layId, float4 rect, float4 uv);

// pushes a quad border
void borderGui(guiContext* ctx, guiLayerId layId, float4 rect, float4 uv);

// pushes elements to make up a text string
void textGui(guiContext* ctx, guiLayerId layId, float2 pos, const char* str);

// pushes an icon
void iconGui(guiContext* ctx, guiLayerId layId, float2 pos, float4 uv);

// pushes a button
int buttonGui(
	guiContext* ctx,
	guiLayerId layId,
	float4 rect,
	float4 ico,
	const char* st
);

// adds a children window
void subWindowGui(guiContext* ctx, window* win);

// pushes an int edit box
void intGui(guiContext* ctx, guiLayerId layId, float4 rect, void* val);

// pushes a float edit box
void floatGui(guiContext* ctx, guiLayerId layId, float4 rect, void* val);

// pushes a string edit box
void stringGui(guiContext* ctx, guiLayerId layId, float4 rect, void* val);

// pushes a 2D vector edit box
void float2Gui(guiContext* ctx, guiLayerId layId, float4 rect, void* val);

// pushes a 3D vector edit box
void float3Gui(guiContext* ctx, guiLayerId layId, float4 rect, void* val);

// pushes a 4D vector edit box
void float4Gui(guiContext* ctx, guiLayerId layId, float4 rect, void* val);

// pushes a 2x2 matrix edit box
void mat2Gui(guiContext* ctx, guiLayerId layId, float4 rect, void* val);

// pushes a 3x3 matrix edit box
void mat3Gui(guiContext* ctx, guiLayerId layId, float4 rect, void* val);

// pushes a 4x4 matrix edit box
void mat4Gui(guiContext* ctx, guiLayerId layId, float4 rect, void* val);

// pushes a transform edit box
void transformGui(guiContext* ctx, guiLayerId layId, float4 rect, void* val);

// pushes a texture edit box
void textureGui(guiContext* ctx, guiLayerId layId, float4 rect, void* val);

// pushes a mesh edit box
void meshGui(guiContext* ctx, guiLayerId layId, float4 rect, void* val);

// pushes a material edit box
void materialGui(guiContext* ctx, guiLayerId layId, float4 rect, void* val);

#endif
