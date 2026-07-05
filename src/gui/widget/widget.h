#ifndef GUI_WIDGET_H
#define GUI_WIDGET_H

#include "../gui.h"
#include "../../math/math.h"
#include "../render/render.h"
#include "../../entity/entity.h"

// -- constants

// size of field entry box
#define FIELD_SIZ 64

// sizing contants
#define PAD 5.0f
#define ROW 60.0f

// atlas sections
#define AU (1.0f / 16)
#define UV(x, y) ((float)(x) * AU), (1.0f - (float)(y) * AU)

// atlas colors
#define BG_ABS   (float4){UV(0, 3), UV(1, 4)}
#define BG_DARK  (float4){UV(1, 3), UV(2, 4)}
#define BG_LIGHT (float4){UV(2, 3), UV(3, 4)}
#define FG_DARK  (float4){UV(3, 3), UV(4, 4)}
#define FG_LIGHT (float4){UV(4, 3), UV(5, 4)}

// atlas icons
#define ICO(x, y) (float4){UV(x, y), UV(x + 1, y + 1)}
#define ICO_ENTITY ICO(0, 4) 
#define ICO_INT    ICO(1, 4) 
#define ICO_FLOAT  ICO(2, 4) 
#define ICO_STR    ICO(3, 4) 
#define ICO_NEW    ICO(0, 5) 
#define ICO_DEL    ICO(1, 5) 

// atlas text sizing
#define TXT_WIDTH  (0.5f * AU * ATLAS_SIZ)
#define TXT_HEIGHT (TXT_WIDTH * 2) 
#define ICO_SIZ   TXT_HEIGHT

// -- input primitives

// test for mouse hover on rect
int hoverGui(guiContext* ctx, float4 rect);

// reads text input
char* bufferGui(
	guiContext* ctx, 
	uint64_t id, 
	float4 rect, 
	int* submit, 
	int* active
);

// -- rendering primitives

// pushes a quad 
void quadGui(guiContext* ctx, float4 rect, float4 uv);

// pushes a quad border 
void borderGui(guiContext* ctx, float4 rect, float4 uv);

// pushes elements to make up a text string
void textGui(guiContext* ctx, float2 pos, const char* str);

// pushes an icon
void iconGui(guiContext* ctx, float2 pos, float4 uv);

// pushes a button
int buttonGui(guiContext* ctx, float4 rect, float4 ico, const char* str);

// creates a new window button
void subWindowGui(
	guiContext* ctx,
	int width,
	int height,
	const char* title,
	renderCallback cback,
	float4 rect,
	float4 ico,
	const char* str
);

// pushes an int edit box  
void intGui(guiContext* ctx, float4 rect, int* val);

// pushes a float edit box  
void floatGui(guiContext* ctx, float4 rect, float* val);

// pushes a string edit box  
void stringGui(guiContext* ctx, float4 rect, char* val);

#endif
