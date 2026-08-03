#include "render.h"
#include "../../render/render.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>

// -- rendering

void pushGui(guiLayer* lay, quad q) {
	// don't overflow
	if(lay->queue.last == QUEUE_SIZ) return;
	lay->queue.vec[lay->queue.last++] = q;
}

// flushes a single GUI layer
void flushLayer(guiContext* ctx, guiLayer* lay) {
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
		sizeof(quad) * lay->queue.last,
		lay->queue.vec
	);
	GL_ERR("gui instance VBO data (flush)")

	// bind quad VAO
	glBindVertexArray(ctx->gl.quadVAO);
	GL_ERR("gui VAO binding (flush)")

	// use shader program
	glUseProgram(ctx->gl.shd->program);

	// bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(
		GL_TEXTURE_2D,
		ctx->gl.tex->tex
	);

	// bind shader uniforms
	glUniform2f(
		glGetUniformLocation(
			ctx->gl.shd->program,
			"uScreenSize"
		),
		ctx->win->width,
		ctx->win->height
	);
	glUniform1i(
		glGetUniformLocation(
			ctx->gl.shd->program,
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

void flushGui(guiContext* ctx) {
	// update child too if present
	if(ctx->child) {
		if(!updateWindow(ctx->child)) {
			freeWindow(ctx->child);
			ctx->child = NULL;
			ctx->inactive = 0;
		}

		// get context back
		glfwMakeContextCurrent(ctx->win->gl);
	}

	// flush all layers
	for(int i = 0; i < GUI_LAYERS; i++) {
		guiLayer* lay = &ctx->layers[i];
		lay->lastHeight = lay->height;
		flushLayer(ctx, lay);
	}
}

// -- initialization

// initializes GUI OpenGL data
int newGui(guiContext* ctx) {
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
		sizeof(quad) * QUEUE_SIZ,
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
		sizeof(quad),
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
		sizeof(quad),
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
	ctx->gl.shd = shaderImport(GUI_VERT_PATH, GUI_FRAG_PATH)->data;
	if(!ctx->gl.shd) return 0;

	// import texture
	ctx->gl.tex = textureImport(GUI_ATLAS_PATH)->data;
	if(!ctx->gl.tex) return 0;

	return 1;
}

void freeGui(void* vCtx) {
	guiContext* ctx = (guiContext*)vCtx;

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
	shaderFree(ctx->gl.shd);

	// free texture
	textureFree(ctx->gl.tex);

	free(ctx);
}

// forward declarations for GLFW callbacks
void charCallback(GLFWwindow* gl, unsigned int codepoint);
void keyCallback(GLFWwindow* gl, int key, int scancode, int action, int mods);
void scrollCallback(GLFWwindow* gl, double x, double y);

guiContext* initGui(window* win) {
	guiContext* ctx = (guiContext*)win->cbak.ctx;

	// make context if NULL 
	if(ctx->win == NULL) {
		if(!newGui(ctx)) return NULL;
		ctx->win = win;

		// attach callbacks
		glfwSetCharCallback(win->gl, charCallback);
		glfwSetKeyCallback(win->gl, keyCallback);
		glfwSetScrollCallback(win->gl, scrollCallback);

		// set other values
		ctx->inactive = 0;
		ctx->in.hotId = 0;
		ctx->in.absScroll = 0.0f;
		ctx->in.dataPtr = NULL;
		ctx->in.dataSet = 0;
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

// -- input

// GLFW character callback
void charCallback(GLFWwindow* gl, unsigned int codepoint) {
	// get context
	window* win = glfwGetWindowUserPointer(gl);
	guiContext* ctx = win->cbak.ctx;

	// check if should input
	if(!ctx 
	|| ctx->in.hotId == 0
	|| ctx->in.keyBufSiz == IN_BUF_SIZ - 1) {
		return;
	}

	// convert to plain ASCII
	ctx->in.keyBuf[ctx->in.keyBufSiz++] = (char)codepoint;
}

// GLFW key callback
void keyCallback(
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
}

// GLFW scroll callback
void scrollCallback(
	GLFWwindow* gl,
	double x __attribute__((unused)),
	double y
) {
	// get context
	window* win = glfwGetWindowUserPointer(gl);
	guiContext* ctx = win->cbak.ctx;

	ctx->in.scroll += (float)y;
}

void inputGui(window* win) {
	guiContext* ctx = (guiContext*)win->cbak.ctx;

	// get mouse position
	double x, y;
	glfwGetCursorPos(win->gl, &x, &y);
	ctx->in.xCur = x;
	ctx->in.yCur = y;

	if(ctx->inactive) {
		ctx->in.curPress = ctx->in.curReles = ctx->in.curDown = 0;
		ctx->in.enter = 0;
		return;
	}

	// get mouse down state
	int down = glfwGetMouseButton(win->gl, GLFW_MOUSE_BUTTON_LEFT);

	// get other mouse states
	ctx->in.curPress = (down  && !ctx->in.prevCur);
	ctx->in.curReles = (!down &&  ctx->in.prevCur);
	ctx->in.curDown  = down;

	// update previous mouse state
	ctx->in.prevCur = down;

	// get key state
	ctx->in.enter  = (glfwGetKey(win->gl, GLFW_KEY_ENTER)  == GLFW_PRESS);
	ctx->in.escape = (glfwGetKey(win->gl, GLFW_KEY_ESCAPE) == GLFW_PRESS);

	// propagate hot state
	if(ctx->in.hotReset) {
		ctx->in.hotReset = 0;
		ctx->in.hotId = 0;
	}
}
