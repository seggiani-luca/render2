#ifndef GUI_WIDGET_H
#define GUI_WIDGET_H

#include "../gui.h"
#include "../../math/math.h"
#include "../render/render.h"
#include "../../entity/entity.h"

// -- constants

// sizing contants
#define PAD 5.0f
#define ROW 60.0f

// atlas sections
#define AU (1.0f / 16)
#define UV(x, y) ((float)(x) * AU), (1.0f - (float)(y) * AU)

// atlas colors
#define BG_DARK  (float4){UV(0, 3), UV(1, 4)}
#define BG_LIGHT (float4){UV(1, 3), UV(2, 4)}
#define FG_DARK  (float4){UV(2, 3), UV(3, 4)}
#define FG_LIGHT (float4){UV(3, 3), UV(4, 4)}

// atlas icons
#define ICO_ENTITY (float4){UV(4, 3), UV(5, 4)} 
#define ICO_INT    (float4){UV(5, 3), UV(6, 4)} 
#define ICO_FLOAT  (float4){UV(6, 3), UV(7, 4)} 
#define ICO_STR    (float4){UV(7, 3), UV(8, 4)} 

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
float quadGui(guiContext* ctx, float4 rect, float4 uv);

// pushes elements to make up a text string
float textGui(guiContext* ctx, float2 rPos, const char* str);

// pushes an icon
void iconGui(guiContext* ctx, float2 rPos, float4 uv);

// pushes an int edit box  
float intGui(guiContext* ctx, float4 rect, int* val);

// pushes a float edit box  
float floatGui(guiContext* ctx, float4 rect, float* val);

// pushes a string edit box  
float stringGui(guiContext* ctx, float4 rect, char* val);

#endif
