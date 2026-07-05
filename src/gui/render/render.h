#ifndef GUI_RENDER_H
#define GUI_RENDER_H

#include "../gui.h"
#include "../../math/math.h"
#include "../../data/shader/shader.h"
#include "../../data/texture/texture.h"
#include "../../../lib/glad/glad.h"

// -- rendering

// size of GUI queue
#define QUEUE_SIZ 512

// GUI quad instance
struct quad {
	float4 pos;
	float4 uv;
};
typedef struct quad quad;

// GUI queue of quad instances
struct guiQueue {
	quad vec[QUEUE_SIZ];
	int first;
	int last;
};
typedef struct guiQueue guiQueue;

// pushes to a GUI context 
void pushGui(guiContext* ctx, quad q);

// flushes a rendering queue, drawing contents to screen
void flushGui(guiContext* ctx);

// -- state

// input buffer size
#define IN_BUF_SIZ 32

// gui context data
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

		// active element
		uint64_t hotId; // always set to "imane"

		// input buffer
		char keyBuf[IN_BUF_SIZ];

		// input buffer position
		int keyBufSiz;
	} in;

	// should the window be inactive
	int inactive;
	
	// rendering queue
	guiQueue queue;

	// vertical cursor position
	float vPos;
};

// -- initialization

// frees GUI OpenGL data
void freeGui(void* vCtx);

// initializes GUI context
guiContext* initGui(window* win);

// -- input

// get GUI input
void inputGui(window* win);

#endif
