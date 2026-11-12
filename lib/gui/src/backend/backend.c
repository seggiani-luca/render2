#include "backend.h"
#include "../gui.h"
#include "../helpers/helpers.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

// select shader paths
#define GUI_VERT_PATH "lib/gui/dat/shader/gui_vert.glsl"
#define GUI_FRAG_PATH "lib/gui/dat/shader/gui_frag.glsl"

// select theme path
#if defined GUI_THEME_DARK
	#define GUI_ATLAS_PATH "lib/gui/dat/texture/gui_atlas_dark.tga"
#elif defined  GUI_THEME_LIGHT
	#define GUI_ATLAS_PATH "lib/gui/dat/texture/gui_atlas_light.tga"
#elif defined  GUI_THEME_NEO
	#define GUI_ATLAS_PATH "lib/gui/dat/texture/gui_atlas_neo.tga"
#elif defined  GUI_THEME_TUX
	#define GUI_ATLAS_PATH "lib/gui/dat/texture/gui_atlas_tux.tga"
#elif defined  GUI_THEME_TOKYO
	#define GUI_ATLAS_PATH "lib/gui/dat/texture/gui_atlas_tokyo.tga"
#else
	#error "No GUI theme set"
#endif

// -- rendering

void guiPush(guiLayer* lay, guiQuad q) {
	// don't overflow
	if(lay->queue.last == QUEUE_SIZ) return;
	lay->queue.vec[lay->queue.last++] = q;
}

// macro for OpenGL errors
#define GL_ERR(func) // empty

// flushes a single GUI layer
void guiFlushLayer(guiContext* ctx, guiLayer* lay) {
	if(lay->queue.last == 0) return;

	// bind GUI quad instance VBO
	glBindBuffer(
		GL_ARRAY_BUFFER,
		ctx->gl.instanceVBO
	);
	GL_ERR("gui instance VBO binding (flush)")

	// push quad data to VBO
	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		sizeof(guiQuad) * lay->queue.last,
		lay->queue.vec
	);
	GL_ERR("gui instance VBO data (flush)")

	// bind quad VAO
	glBindVertexArray(ctx->gl.quadVAO);
	GL_ERR("gui VAO binding (flush)")

	// use shader program
	glUseProgram(ctx->gl.shd);

	// bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(
		GL_TEXTURE_2D,
		ctx->gl.tex
	);

	// bind shader uniforms
	glUniform2f(
		glGetUniformLocation(
			ctx->gl.shd,
			"uScreenSize"
		),
		ctx->win->width,
		ctx->win->height
	);
	glUniform1i(
		glGetUniformLocation(
			ctx->gl.shd,
			"uTex"
		),
		0
	);

	// issue draw call
	glDrawArraysInstanced(
		GL_TRIANGLES,
		0,
		6,
		lay->queue.last
	);
	GL_ERR("gui drawing")
}

void guiFlush(guiContext* ctx) {
	// good time to also clear input
	ctx->in.curReles = ctx->in.curPress = 0;
	ctx->in.escape = ctx->in.enter = 0;
	ctx->in.scroll = 0.0f;

	// clear buffer
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// flush all layers
	for(int i = 0; i < GUI_LAYERS; i++) {
		guiLayer* lay = &ctx->layers[i];
		lay->lastHeight = lay->height;
		guiFlushLayer(ctx, lay);
	}
}

// -- initialization

// initializes GUI OpenGL data
int guiNew(guiContext* ctx) {
	float quadTris[] =
	{
		0.0f, 0.0f, // tri 0
		1.0f, 0.0f,
		1.0f, 1.0f,

		0.0f, 0.0f, // tri 1
		1.0f, 1.0f,
		0.0f, 1.0f,
	};

	// initialize basic quad VAO
	glGenVertexArrays(
		1,
		&ctx->gl.quadVAO
	);
	GL_ERR("gui VAO generation")
	glBindVertexArray(ctx->gl.quadVAO);
	GL_ERR("gui VAO binding")

	// initialize basic quad VBO
	glGenBuffers(
		1,
		&ctx->gl.quadVBO
	);
	GL_ERR("gui VBO generation")
	glBindBuffer(
		GL_ARRAY_BUFFER,
		ctx->gl.quadVBO
	);
	GL_ERR("gui VBO binding")

	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(quadTris),
		quadTris,
		GL_STATIC_DRAW
	);
	GL_ERR("gui VBO data")

	// quad vertex attribute
	glVertexAttribPointer(
		0,
		2,
		GL_FLOAT,
		GL_FALSE,
		2 * sizeof(float),
		(void*)0
	);
	GL_ERR("gui vertex attrib")
	glEnableVertexAttribArray(0);
	GL_ERR("gui vertex attrib enable")

	// initialize GUI quad instance VBO
	glGenBuffers(
		1,
		&ctx->gl.instanceVBO
	);
	GL_ERR("gui instance VBO generation")
	glBindBuffer(
		GL_ARRAY_BUFFER,
		ctx->gl.instanceVBO
	);
	GL_ERR("gui instance VBO binding")

	// fill quad instance VBO with empty data
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(guiQuad) * QUEUE_SIZ,
		NULL,
		GL_DYNAMIC_DRAW
	);
	GL_ERR("gui instance VBO data init")

	// quad bounds (x, y, w, h) attribute
	glVertexAttribPointer(
		1,
		4,
		GL_FLOAT,
		GL_FALSE,
		sizeof(guiQuad),
		(void*)0
	);
	GL_ERR("gui instance vertex attrib")
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(
		1,
		1
	);
	GL_ERR("gui instance vertex attrib enable")

	// quad UV attribute
	glVertexAttribPointer(
		2,
		4,
		GL_FLOAT,
		GL_FALSE,
		sizeof(guiQuad),
		(void*)(4 * sizeof(float))
	);
	GL_ERR("gui instance UV attrib")
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(
		2,
		1
	);
	GL_ERR("gui instance UV attrib enable")

	// import shader
	ctx->gl.shd = guiImportShader(GUI_VERT_PATH, GUI_FRAG_PATH);
	if(!ctx->gl.shd) {
		glDeleteBuffers(
			1,
			&ctx->gl.quadVBO
		);
		glDeleteBuffers(
			1,
			&ctx->gl.instanceVBO
		);
		glDeleteVertexArrays(
			1,
			&ctx->gl.quadVAO
		);

		return 0;
	}

	// import texture
	ctx->gl.tex = guiImportTexture(GUI_ATLAS_PATH);
	if(!ctx->gl.tex) {
		glDeleteBuffers(
			1,
			&ctx->gl.quadVBO
		);
		glDeleteBuffers(
			1,
			&ctx->gl.instanceVBO
		);
		glDeleteVertexArrays(
			1,
			&ctx->gl.quadVAO
		);
		guiFreeShader(ctx->gl.shd);

		return 0;
	}

	return 1;
}

void guiFree(void* vCtx) {
	guiContext* ctx = (guiContext*)vCtx;
	
	// free child first if present
	if(ctx->child) winFree(ctx->child);

	// free VBOs and VAO
	glDeleteBuffers(
		1,
		&ctx->gl.quadVBO
	);
	glDeleteBuffers(
		1,
		&ctx->gl.instanceVBO
	);
	glDeleteVertexArrays(
		1,
		&ctx->gl.quadVAO
	);

	// free shader
	guiFreeShader(ctx->gl.shd);

	// free texture
	guiFreeTexture(ctx->gl.tex);

	free(ctx);
}

// -- input

// GLFW character callback
void guiCharCallback(GLFWwindow* gl, unsigned int codepoint) {
	// get context
	window* win = glfwGetWindowUserPointer(gl);
	guiContext* ctx = win->cbak.ctx;

	// check if should input
	if(!ctx 
	|| ctx->in.hotId == 0
	|| ctx->in.keyBufSiz == GUI_IN_BUF_SIZ - 2) {
		return;
	}

	// convert to plain ASCII
	ctx->in.keyBuf[ctx->in.keyBufSiz++] = (char)codepoint;
}

// GLFW key callback
void guiKeyCallback(
	GLFWwindow* gl,
	int key,
	int scancode __attribute__((unused)),
	int action __attribute__((unused)),
	int mods __attribute__((unused))
) {
	// get context
	window* win = glfwGetWindowUserPointer(gl);
	guiContext* ctx = win->cbak.ctx;

	// go back on backspace
	if(key == GLFW_KEY_BACKSPACE 
	&& action == GLFW_PRESS 
	&& ctx->in.keyBufSiz > 0) {
		ctx->in.keyBuf[--ctx->in.keyBufSiz] = '\0';
	}

	// register enter
	if(key == GLFW_KEY_ENTER) {
		ctx->in.enter = (action == GLFW_PRESS);
	}

	// register escape
	if(key == GLFW_KEY_ESCAPE) {
		ctx->in.escape = (action == GLFW_PRESS);
	}
}

// GLFW scroll callback
void guiScrollCallback(
	GLFWwindow* gl,
	double x __attribute__((unused)),
	double y
) {
	// get context
	window* win = glfwGetWindowUserPointer(gl);
	guiContext* ctx = win->cbak.ctx;

	ctx->in.scroll += y;
}

// GLFW mouse callback
void guiMouseCallback(
	GLFWwindow* gl,
	double x,
	double y
) {
    window* win = glfwGetWindowUserPointer(gl);
    guiContext* ctx = win->cbak.ctx;

	ctx->in.xCur = WIN_USE_WAYLAND ? x : fbToWinW(win, x);
	ctx->in.yCur = WIN_USE_WAYLAND ? y : fbToWinH(win, y);
}

// GLFW mouse button callback
void guiMouseButtonCallback(
    GLFWwindow* gl,
	int button,
	int action,
	int mods __attribute__((unused))
) {
    window* win = glfwGetWindowUserPointer(gl);
    guiContext* ctx = win->cbak.ctx;

    if(button != GLFW_MOUSE_BUTTON_LEFT || ctx->inactive)
        return;

	if(action == GLFW_PRESS) {
		ctx->in.curPress = 1;
		ctx->in.curReles = 0;
		ctx->in.curDown = 1;
	}
	if(action == GLFW_RELEASE) {
		ctx->in.curPress = 0;
		ctx->in.curReles = 1;
		ctx->in.curDown = 0;
	}
}

// currently focused window
static window* curFocus = NULL;

void guiInput(window* win) {
	guiContext* ctx = (guiContext*)win->cbak.ctx;

	// reset all on inactive window
	if(ctx->inactive) {
		ctx->in.curPress = ctx->in.curReles = ctx->in.curDown = 0;
		ctx->in.enter = 0;
		return;
	}

	// use focus state
	if(ctx->in.curPress) curFocus = win;
	if(win != curFocus) {
		ctx->in.curPress = 0;
		ctx->in.curReles = 0;
		ctx->in.curDown = 0;
	}

	// propagate hot state
	if(ctx->in.hotReset) {
		ctx->in.hotReset = 0;
		ctx->in.hotId = 0;
	}
}

guiContext* guiInit(window* win) {
	guiContext* ctx = (guiContext*)win->cbak.ctx;

	// make context if NULL 
	if(ctx->win == NULL) {
		if(!guiNew(ctx)) return NULL;
		ctx->win = win;

		// attach callbacks
		glfwSetCharCallback(win->gl, guiCharCallback);
		glfwSetKeyCallback(win->gl, guiKeyCallback);
		glfwSetScrollCallback(win->gl, guiScrollCallback);
		glfwSetCursorPosCallback(win->gl, guiMouseCallback);
		glfwSetMouseButtonCallback(win->gl, guiMouseButtonCallback);

		// reset other values 
		ctx->in.curDown = ctx->in.curPress = ctx->in.curDown = 0;
		ctx->in.escape = ctx->in.enter = 0;
		ctx->inactive = 0;
		ctx->in.hotId = 0;
		ctx->in.hotReset = 0;
		ctx->in.absScroll = 0.0f;
		ctx->in.dataPtr = NULL;
		ctx->in.dataSet = 0;
	}

	// update child first if present
	if(ctx->child) {
		if(!winUpdate(ctx->child)) {
			winFree(ctx->child);
			ctx->child = NULL;
			ctx->inactive = 0;
		}

		// get context back
		glfwMakeContextCurrent(ctx->win->gl);
	}

	// reset all layers
	for(int i = 0; i < GUI_LAYERS; i++) {
		guiLayer* lay = &ctx->layers[i];

		// reset queue and cursor
		lay->queue.last = 0;
		lay->vPos = 0.0f;
		lay->height = 0.0f;
	}

	return ctx;
}
