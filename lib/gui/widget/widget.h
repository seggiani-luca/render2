#ifndef GUI_WIDGET_H
#define GUI_WIDGET_H

#include "../gui.h"
#include "../render/render.h"

// -- constants

// sizing contants
#define PAD        * 4.0f           // padding between elements
#define HPAD         (1 PAD / 2.0f) // half padding
#define ROW          48.0f          // size of row layout elements 
#define HROW         (ROW / 2.0f)   // half row
#define BORDER       2.0f           // thickness of element border
#define ICO_SIZ      TXT_HEIGHT     // size of icon bounding box
#define ICO_OFFSET   2.0f           // size gain of icon inside bounding box

// atlas text sizing
#define TXT_WIDTH  (0.5f * AU * ATLAS_SIZ) // width of glyph 
#define TXT_HEIGHT (TXT_WIDTH * 2)         // height of glyph

// window dependent sizing constants
#define WIN  (ctx->win->width)        // width of window
#define HWIN (ctx->win->width / 2.0f) // half of width
#define HEIG (ctx->win->height)       // height of window

// atlas sizing unit
#define AU       (1.0f / ATLAS_DIVISION)

// atlas colors
#define BG_ABS   (rectangle) { UV(0.5, 3.5), UV(0.5, 3.5) }
#define BG_DARK  (rectangle) { UV(1.5, 3.01), UV(1.5, 3.99) }
#define BG_LIGHT (rectangle) { UV(2.5, 3.5), UV(2.5, 3.5) }
#define FG_DARK  (rectangle) { UV(3.5, 3.5), UV(3.5, 3.5) }
#define FG_LIGHT (rectangle) { UV(4.5, 3.5), UV(4.5, 3.5) }

// returns UVs of a square selection from the atlas
#define UV(x, y) ((float)(x) * AU), (1.0f - (float)(y) * AU)

// selects an icon (a single square element) from the atlas 
#define ICO(x, y)  (rectangle) { UV(x, y), UV(x + 1,  y + 1) }

// -- input primitives

// test for mouse hover on rectangle
int hoverGui(guiContext* ctx, guiLayerId layId, rectangle rect);

// test for mouse release on rectangle
int relesGui(guiContext* ctx, guiLayerId layId, rectangle rect);

// test for mouse press on rectangle
int pressGui(guiContext* ctx, guiLayerId layId, rectangle rect);

// reads text input
char* bufferGui(
	guiContext* ctx,
	guiLayerId layId,
	uint64_t id,
	rectangle rect,
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
void quadGui(guiContext* ctx, guiLayerId layId, rectangle rect, rectangle uv);

// pushes a quad border
void borderGui(
	guiContext* ctx,
	guiLayerId layId,
	rectangle rect,
	rectangle uv
);

// pushes elements to make up a text string, up to a length
void textGuiN(
	guiContext* ctx,
	guiLayerId layId,
	position pos,
	const char* str,
	size_t len
);

// pushes elements to make up a text string
void textGui(guiContext* ctx, guiLayerId layId, position pos, const char* str);

// pushes elements to make up a text separator 
void separatorGui(
	guiContext* ctx,
	guiLayerId layId,
	rectangle rect,
	const char* str
);

// pushes an icon
void iconGui(guiContext* ctx, guiLayerId layId, position pos, rectangle uv);

// pushes a button
int buttonGui(
	guiContext* ctx,
	guiLayerId layId,
	rectangle rect,
	rectangle ico,
	const char* st
);

// pushes a non-interactive text box
void textBoxGui(
	guiContext* ctx,
	guiLayerId layId,
	rectangle rect,
	const char* str
);

// pushes an interactive text box
char* editBoxGui(
	guiContext* ctx,
	guiLayerId layId,
	rectangle rect,
	const char* str,
	uint64_t id
);

// adds a children window
void subWindowGui(guiContext* ctx, window* win);

// -- example widgets

// pushes an int edit box
int intGui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val);

// pushes a float edit box
int floatGui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val);

// pushes a string edit box
int stringGui(guiContext* ctx, guiLayerId layId, rectangle rect, void* val);

#endif
