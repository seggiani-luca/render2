#ifndef GUI_WIDGET_H
#define GUI_WIDGET_H

#include "../backend/backend.h"

// -- local constants: change these to change GUI behavoir 

/**
 * Size gain of icons inside their bounding box.
 */
#define GUI_ICO_OFFSET 2.0f

/**
 * Atlas sizing unit, corresponds to one GUI atlas division in the 0-1 range.
 */
#define GUI_AU (1.0f / GUI_ATLAS_DIVISION)

/**
 * Width of text characters in the GUI atlas. The default value is half of an
 * icon.
 */
#define GUI_TXT_WIDTH (0.5f * GUI_AU * GUI_ATLAS_SIZ)

/**
 * Height of text characters. The default setting allows for fonts with a 1:2
 * aspect ratio (as with plenty of terminal based monospace fonts).
 */
#define GUI_TXT_HEIGHT (GUI_TXT_WIDTH * 2)

/**
 * Produces the corresponding UV position of (x, y) integer coordinates on a
 * GUI atlas. To be used inside an initializer expression, as in:
 *
 * position uv = (position){ WIN_UV(1, 2) };
 *
 */
#define GUI_UV(x, y) ((float)(x) * GUI_AU), (1.0f - (float)(y) * GUI_AU)

/**
 * Produces the UV rectangle needed to select an icon (a single square element)
 * from a GUI atlas. 
 */
#define GUI_ICO(x, y) (guiRect) { GUI_UV(x, y), GUI_UV(x + 1,  y + 1) }

/**
 * Mouse scrolling sensitivity for scrollable layers.
 */
#define GUI_SCROLL_SENSITIVITY 4.0f

// -- user facing constants: use these to create new GUI elements 

/**
 * Padding between GUI elements. The weird syntax allows for things like:
 * 
 * float pad = 3 PAD + 5 PAD;
 */
#define GUI_PAD * 4.0f

/**
 * Half of the padding size.
 */
#define GUI_HPAD (1 GUI_PAD / 2.0f)

/**
 * Size of row layout elements.
 */
#define GUI_ROW 48.0f

/**
 * Half of the row size.
 */
#define GUI_HROW (GUI_ROW / 2.0f)

/**
 * Thickness of element border. A value close to 1 pixel is recommended (the
 * default value of 2 pixels displays better on HiDPI screens).
 */
#define GUI_BORDER 2.0f

/**
 * Size of icon bounding box. Icons are allowed to grow / shrink inside their
 * bounding box.
 */
#define GUI_ICO_SIZ GUI_TXT_HEIGHT

/**
 * Absolute background color.
 */
#define GUI_BG_ABS (guiRect) { GUI_UV(0.5, 3.5), GUI_UV(0.5, 3.5) }

/**
 * Dark background color.
 */
#define GUI_BG_DARK (guiRect) { GUI_UV(1.5, 3.5), GUI_UV(1.5, 3.5) }

/**
 * Light background color.
 */
#define GUI_BG_LIGHT (guiRect) { GUI_UV(2.5, 3.5), GUI_UV(2.5, 3.5) }

/**
 * Dark foreground color.
 */
#define GUI_FG_DARK (guiRect) { GUI_UV(3.5, 3.5), GUI_UV(3.5, 3.5) }

/**
 * Light foreground color.
 */
#define GUI_FG_LIGHT (guiRect) { GUI_UV(4.5, 3.5), GUI_UV(4.5, 3.5) }

/**
 * Width of the current window, extracted from the window library.
 */
#define WIN_WIDTH (ctx->win->width) 

/**
 * Half of the width of the current window.
 */
#define HWIN_WIDTH (ctx->win->width / 2.0f)

/**
 * Height of the current window, extracted from the window library.
 */
#define WIN_HEIGHT (ctx->win->height)


// -- input primitives

/**
 * Tests for mouse hover on a rectangular region.
 *
 * @param ctx       GUI context to act on
 * @param layId     layer ID of layer to act on
 * @param rect      rectangle to check
 * @return          is mouse hovering?
 */
int guiHover(guiContext* ctx, guiLayerId layId, guiRect rect);

/**
 * Tests for mouse press on a rectangular region. Also consumes other mouse
 * events.
 *
 * @param ctx       GUI context to act on
 * @param layId     layer ID of layer to act on
 * @param rect      rectangle to check
 * @return          was mouse released?
 */
int guiRelease(guiContext* ctx, guiLayerId layId, guiRect rect);

/**
 * Tests for mouse release on a rectangular region. Also consumes other mouse
 * events.
 *
 * @param ctx       GUI context to act on
 * @param layId     layer ID of layer to act on
 * @param rect      rectangle to check
 * @return          was mouse pressed?
 */
int guiPress(guiContext* ctx, guiLayerId layId, guiRect rect);

/**
 * Reads text input to the local buffer when a rectangular region is pressed.
 * Can be used to implement editable text boxes.
 *
 * @param ctx    GUI context to act on
 * @param layId  layer ID of layer to act on
 * @param id     an unique id which must be provided to be associated with
 *               the object being edited (if heap allocated, the object
 *               address can suffice)
 * @param rect   rectangle to check
 * @param submit flag that signals if the editing operation finished
 * @param active flag that signals if an editing operation is in progress
 */
char* guiBuffer(
	guiContext* ctx,
	guiLayerId layId,
	uint64_t id,
	guiRect rect,
	int* submit,
	int* active
);

/**
 * Scrolls a GUI layer up to visible contents.
 *
 * @param ctx   GUI context to act on
 * @param layId layer ID of layer to scroll 
 */
void guiScroll(guiContext* ctx, guiLayerId layId);

// -- rendering primitives

/**
 * Moves the internal GUI cursor down bu a given amount. Can be used for
 * manual vertical layout groups.
 *
 * @param ctx   GUI context to act on
 * @param layId layer ID of layer to act on
 * @param amt   vertical space to insert
 */
void guiDown(guiContext* ctx, guiLayerId layId, float amt);

/**
 * Trims a GUI layer to visible contents.
 *
 * @param ctx   GUI context to act on
 * @param layId layer ID of layer to trim to 
 */
void guiTrim(guiContext* ctx, guiLayerId layId);

/**
 * Pushes a visible quad to a GUI layer.
 *
 * @param ctx       GUI context to act on
 * @param layId     layer ID of layer to act on
 * @param rect      rectangle of quad
 * @param uv        UVs used to sample the GUI atlas for quad contents
 */
void guiPushQuad(guiContext* ctx, guiLayerId layId, guiRect rect, guiRect uv);

/**
 * Pushes a border around a quad to a GUI layer.
 *
 * @param ctx       GUI context to act on
 * @param layId     layer ID of layer to act on
 * @param rect      rectangle of border 
 * @param uv        UVs used to sample the GUI atlas for border contents (using
 *                  a flat is recommended to avoid artifacts) 
 */
void guiBorder(
	guiContext* ctx,
	guiLayerId layId,
	guiRect rect,
	guiRect uv
);

/**
 * Pushes elements to make up a text string, up to a given length.
 *
 * @param ctx   GUI context to act on
 * @param layId layer ID of layer to act on
 * @param pos   position of text on the layer
 * @param str   string of text to display
 * @param len   length of text string
 */
void guiTextN(
	guiContext* ctx,
	guiLayerId layId,
	guiPos pos,
	const char* str,
	size_t len
);

/**
 * Pushes elements to make up a text string, stopping when the termnator is
 * found.
 *
 * @param ctx   GUI context to act on
 * @param layId layer ID of layer to act on
 * @param pos   position of text on the layer
 * @param str   string of text to display
 */
void guiText(guiContext* ctx, guiLayerId layId, guiPos pos, const char* str);

/**
 * Pushes elements to make up a text separator, that is an element comprised of
 * a text label and an horizontal line break.
 *
 * @param ctx   GUI context to act on
 * @param layId layer ID of layer to act on
 * @param pos   position of text on the layer
 * @param str   string of text to display
 */
void guiSeparator(
	guiContext* ctx,
	guiLayerId layId,
	guiRect rect,
	const char* str
);

/**
 * Pushes an icon. 
 *
 * @param ctx   GUI context to act on
 * @param layId layer ID of layer to act on
 * @param pos   position of icon on the layer
 * @param uv    UVs used to sample the GUI atlas for the icon 
 */
void guiIcon(guiContext* ctx, guiLayerId layId, guiPos pos, guiRect uv);

/**
 * Pushes a button. 
 *
 * @param ctx   GUI context to act on
 * @param layId layer ID of layer to act on
 * @param rect  rectangle of button 
 * @param ico   UVs used to sample the GUI atlas for the icon
 * @param st    (optional) label of button
 * @return      was the button pressed?
 */
int guiButton(
	guiContext* ctx,
	guiLayerId layId,
	guiRect rect,
	guiRect ico,
	const char* st
);

/**
 * Pushes a non-editable text box. 
 *
 * @param ctx   GUI context to act on
 * @param layId layer ID of layer to act on
 * @param rect  rectangle of box 
 * @param str   string to display 
 */
void guiTextBox(
	guiContext* ctx,
	guiLayerId layId,
	guiRect rect,
	const char* str
);

/**
 * Pushes an editable text box. 
 *
 * @param ctx   GUI context to act on
 * @param layId layer ID of layer to act on
 * @param rect  rectangle of box 
 * @param str   string to edit
 * @return      pointer to edited buffer
 */
char* guiEditBox(
	guiContext* ctx,
	guiLayerId layId,
	guiRect rect,
	const char* str,
	uint64_t id
);

/**
 * Assigns a child window to a GUI context, deactivating the context until the
 * child window is alive.
 *
 * @param ctx GUI context to act on
 * @param win window to add as child window
 */
void guiChildWindow(guiContext* ctx, window* win);

// -- example widgets

/**
 * Pushes an integer edit box.
 *
 * @param ctx   GUI context to act on
 * @param layId layer ID of layer to act on
 * @param rect  rectangle of box
 * @param val   pointer to the value to edit
 * @return      was the value edited?
 */
int intGui(guiContext* ctx, guiLayerId layId, guiRect rect, void* val);

/**
 * Pushes a float edit box.
 *
 * @param ctx   GUI context to act on
 * @param layId layer ID of layer to act on
 * @param rect  rectangle of box
 * @param val   pointer to the value to edit
 * @return      was the value edited?
 */
int floatGui(guiContext* ctx, guiLayerId layId, guiRect rect, void* val);

/**
 * Pushes a string edit box.
 *
 * @param ctx   GUI context to act on
 * @param layId layer ID of layer to act on
 * @param rect  rectangle of box
 * @param val   pointer to the value to edit
 * @return      was the value edited?
 */
int stringGui(guiContext* ctx, guiLayerId layId, guiRect rect, void* val);

#endif
