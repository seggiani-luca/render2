#ifndef GUI_RENDER_H
#define GUI_RENDER_H

#include "../gui.h"
#include "../../../lib/glad/glad.h"
#include "../../data/shader/shader.h"
#include "../../data/texture/texture.h"

// -- input

// size of input buffer, that is data entered by user 
#define IN_BUF_SIZ DAT_PATH_SIZ

// size of output buffer, that is display buffers for GUI
#define OUT_BUF_SIZ 64

// -- rendering

// macro for OpenGL errors
#define GL_ERR(func)                                    \
	{                                                   \
	    GLenum err = glGetError();                      \
	    if(err != GL_NO_ERROR)                          \
	    printf("OpenGL error 0x%x at %s\n", err, func); \
	}

// GUI coordinate 
struct position {
	// position 
	float x; float y;
};
typedef struct position position;

// GUI rectangle
struct rectangle {
	// top left corner position
	float x; float y;

	// width
	float w;

	// height
	float h;
};
typedef struct rectangle rectangle;

// GUI quad instance
struct quad {
	rectangle pos;
	rectangle uv;
};
typedef struct quad quad;

// size of GUI queue
#define QUEUE_SIZ 2048

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
		char keyBuf[IN_BUF_SIZ];

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
