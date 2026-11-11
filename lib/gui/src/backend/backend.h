#ifndef GUI_BACKEND_H
#define GUI_BACKEND_H

#include "../../../window/window.h"
#include "../../../../lib/glad/glad.h"
#include <limits.h>

// -- atlas 

/*
 * Structure of a GUI atlas:
 *
 * row 0-2:  ASCII character set in standard order;
 * row 3:    Background colors:
 *           - Absolute background (pitch black / white)
 *           - Dark background (controls, fields)
 *           - Light background (panels)
 *           - Dark foreground (borders)
 *           - Light foreground (text, icons)
 * row 4-15: Icons, to be defined by use case.
 *
 * Examples are provided in the dat/ directory.
 */

/**
 * Size of atlas grid. Defaults to 16 (as in the examples).
 */
#define GUI_ATLAS_DIVISION 16

/**
 * Pixel size of the atlas. Doesn't have to correspond to actual GUI size.
 */
#define GUI_ATLAS_DIM 1024

/**
 * Scaling of GUI, applied to the pixel size of the atlas.
 */
#define GUI_GUI_SCALE 0.25f

/**
 * On-screen size of the atlas, calculated from pixel size and GUI scaling. 
 */
#define GUI_ATLAS_SIZ (GUI_ATLAS_DIM * GUI_GUI_SCALE)

// -- input

// size of input buffer, that is data entered by user
/**
 * Size of the input buffer, that is the buffer used to mantain data entered by
 * the user.
 */
#define GUI_IN_BUF_SIZ PATH_MAX

/**
 * Size of the output buffer, that is the buffer used to display data back to
 * the user. Can be smaller than IN_BUF_SIZ as the value is stored in memory
 * before display (maybe bad UX, but no lost data).
 */
#define GUI_OUT_BUF_SIZ 64

// -- rendering

/**
 * Macro for OpenGL error printing. 
 */
#define GL_ERR(func)                                    \
	{                                                   \
	    GLenum err = glGetError();                      \
	    if(err != GL_NO_ERROR)                          \
	    printf("OpenGL error 0x%x at %s\n", err, func); \
	}

/**
 * Struct for GUI coordinates.
 */
struct guiPos {
	// position 
	float x; float y;
};
typedef struct guiPos guiPos;

/**
 * Struct for GUI rectangles, defined by top-left position, width and height.
 */
struct guiRect {
	// top left corner position
	float x; float y;

	// width
	float w;

	// height
	float h;
};
typedef struct guiRect guiRect;

/**
 * Instance of a GUI quad. A quad is the combination of a rectangle on a window
 * and the rectangle to map to it from the atlas. 
 */
struct guiQuad {
	// window rectangle
	guiRect rect;

	// atlas rectangle
	guiRect uv;
};
typedef struct guiQuad guiQuad;

/**
 * Size of a GUI quad queue.
 */
#define QUEUE_SIZ 2048

/**
 * A GUI quad queue, that is a list of GUI quads which gets assembled as window
 * contents are built, and then rendered on a window.
 */
typedef struct {
	// vector of pushed GUI quads
	guiQuad vec[QUEUE_SIZ];

	// index of next GUI quad to push
	int last;
} guiQueue;

// GUI layer (rendering queue and cursor position)
/**
 * A GUI layer, that is a combination of the following:
 * - A GUI queue, which is to be rendered to a window;
 * - The vertical cursor position of the layer, which the user is expected to
 *   use to keep track of vertical layout. Scroll commands might change this;
 * - The current height of the layer, which gets updated alongside the vertical
 *   cursor position. Scroll commands never change this;
 * - The height of the layer at the last frame. 
 */
struct guiLayer {
	// rendering queue of quad instances
	guiQueue queue;

	// vertical cursor position
	float vPos;

	// layer height 
	float height;
	
	// last frame render layer height
	float lastHeight;
};
typedef struct guiLayer guiLayer;

/**
 * Enum for GUI layer types. These get rendered top to bottom, meaning FIXED
 * appears above SCROLL, which appears above BACKGROUND.
 */
typedef enum {
	BACKGROUND,
	SCROLL,
	FIXED,
	GUI_LAYERS // number of guiLayerIds
} guiLayerId;

/**
 * A GUI context. This groups information about a GUI, including:
 * - Its' window;
 * - It's child window (if present);
 * - OpenGL specific context;
 * - GUI input context;
 * - GUI layers;
 * - An active flag for the whole GUI window.
 */
struct guiContext {
	// window handle (also used as validity flags for the context's
	// initialization state)
	window* win;

	// children window handle
	window* child;

	// substruct for OpenGL specific context
	struct {
		// VAO for basic quad
		GLuint quadVAO;

		// VBO for basic quad
		GLuint quadVBO;

		// VBO for GUI quad instances
		GLuint instanceVBO;

		// GUI shader
		GLuint shd;

		// GUI texture
		GLuint tex;
	} gl;

	// substruct for GUI input context
	struct {
		// mouse X and Y
		double xCur, yCur;

		// mouse state (1 if pressed, 0 otherwise) 
		int curDown;

		// mouse state rising edge
		int curPress;

		// mouse state falling edge;
		int curReles;

		// previous mouse state
		int prevDown;

		// enter key state
		int enter;

		// escape key state
		int escape;

		// active element
		uint64_t hotId; // always set to "imane"

		// should the active element be reset at the next frame?
		int hotReset;

		// input buffer
		char keyBuf[GUI_IN_BUF_SIZ];

		// input buffer position
		int keyBufSiz;

		// scroll state
		float scroll;

		// absolute scrolling position
		float absScroll;

		// editable data pointer
		void* dataPtr;

		// was editable data modified? 
		int dataSet;
	} in;

	// GUI layers
	guiLayer layers[GUI_LAYERS];

	// inactive GUI window flag 
	int inactive;
};
typedef struct guiContext guiContext;

// -- rendering

/**
 * Pushes a quad to a GUI layer.
 *
 * @param lay GUI layer to act on
 * @param q   quad to push
 */
void guiPush(guiLayer* lay, guiQuad q);

/**
 * Initializes a GUI context for rendering on a window (to be called each time
 * the window is updated, so each frame).
 *
 * @param  win window context belongs to (needed to fully initialize context)
 * @return the fully initialized, ready context 
 */
guiContext* guiInit(window* win);

/**
 * Flushes all the rendering queues of a context, drawing contents to the GUI
 * window.
 *
 * @param ctx GUI context to act on
 */
void guiFlush(guiContext* ctx);

// -- initialization

/**
 * Frees a GUI's data (mainly OpenGL objects). Setting this to a window close
 * callback completely removes manual frees from user code).
 *
 * @param ctx GUI context to free 
 */
void guiFree(void* vCtx);

// -- input

/**
 * Updates a GUI window's input state.
 *
 * @param win window context belongs to
 */
void guiInput(window* win);

#endif
