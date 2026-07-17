#ifndef GUI_RENDER_H
#define GUI_RENDER_H

#include "../../../lib/glad/glad.h"
#include "../../data/shader/shader.h"
#include "../../data/texture/texture.h"
#include "../../math/math.h"
#include "../gui.h"

// -- rendering

// GUI quad instance
struct quad {
	float4 pos;
	float4 uv;
};
typedef struct quad quad;

// size of GUI queue
#define QUEUE_SIZ 2048

// input buffer size
#define IN_BUF_SIZ 32

// GUI queue
typedef struct {
	quad vec[QUEUE_SIZ];
	int last;
} guiQueue;

// GUI layer (rendering queue and cursor position)
typedef struct {
	// rendering queue of quad instances
	guiQueue queue;

	// vertical cursor position
	float vPos;

	// last render layer height
	float lastHeight;

	// layer height 
	float height;
} guiLayer;

typedef enum {
	BACKGROUND,
	SCROLL,
	FIXED,
	GUI_LAYERS
} guiLayerId;

// GUI context data
struct guiContext {
	// window handle
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
		shader* shd;

		// GUI atlas texture
		texture* tex;
	} gl;

	// substruct for GUI input state
	struct {
		// mouse X and Y
		double xCur, yCur;

		// mouse state
		int curDown;

		// mouse state rising edge
		int curPress;

		// mouse state falling edge;
		int curReles;

		// previous mouse state
		int prevCur;

		// enter key state
		int enter;

		// escape key state
		int escape;

		// active element
		uint64_t hotId; // always set to "imane"

		// should the active element be reset at the next frame?
		int hotReset;

		// input buffer
		char keyBuf[IN_BUF_SIZ];

		// input buffer position
		int keyBufSiz;

		// scroll state
		float scroll;

		// absolute scrolling position
		float absScroll;
	} in;

	// GUI layers
	guiLayer layers[GUI_LAYERS];

	// should the window be inactive
	int inactive;
};

// -- rendering

// pushes to a GUI context
void pushGui(guiLayer* ctx, quad q);

// initializes GUI context
guiContext* initGui(window* win);

// flushes a rendering queue, drawing contents to screen
void flushGui(guiContext* ctx);

// -- initialization

// frees GUI OpenGL data
void freeGui(void* vCtx);

// -- input

// get GUI input
void inputGui(window* win);

#endif
