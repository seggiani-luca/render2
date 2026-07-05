#include "render.h"
#include "../../render/render.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>

// -- rendering

void pushGui(guiContext* ctx, quad q) {
	// don't overflow
	if(ctx->queue.last == QUEUE_SIZ) return;
	ctx->queue.vec[ctx->queue.last++] = q;
}

void flushGui(guiContext* ctx) {
	if(ctx->queue.last == 0) return;

	// bind GUI quad instance VBO
	glBindBuffer(GL_ARRAY_BUFFER, ctx->gl.instanceVBO);
	GL_ERR("gui instance VBO binding (flush)")

	// push quad data to VBO
	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		sizeof(quad) * ctx->queue.last,
		ctx->queue.vec
	);
	GL_ERR("gui instance VBO data (flush)")

	// bind quad VAO
	glBindVertexArray(ctx->gl.quadVAO);
	GL_ERR("gui VAO binding (flush)")

	// use shader program
	glUseProgram(ctx->gl.shd->program);

	// bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ctx->gl.tex->tex);
	
	// bind shader uniforms
	glUniform2f(
		glGetUniformLocation(ctx->gl.shd->program, "uScreenSize"),
		ctx->win->width,
		ctx->win->height
	);
	glUniform1i(glGetUniformLocation(ctx->gl.shd->program, "uTex"), 0);

	// issue draw call
	glDrawArraysInstanced(
		GL_TRIANGLES,
		0,
		6,
		ctx->queue.last
	);
	GL_ERR("gui drawing")
}

// -- initialization

// initializes GUI OpenGL data
int newGui(guiContext* ctx) {
	float quadTris[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,

		0.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
	};

	// initialize basic quad VAO
	glGenVertexArrays(1, &ctx->gl.quadVAO);
	GL_ERR("gui VAO generation")
	glBindVertexArray(ctx->gl.quadVAO);
	GL_ERR("gui VAO binding")

	// initialize basic quad VBO
	glGenBuffers(1, &ctx->gl.quadVBO);
	GL_ERR("gui VBO generation")
	glBindBuffer(GL_ARRAY_BUFFER, ctx->gl.quadVBO);
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
	glGenBuffers(1, &ctx->gl.instanceVBO);
	GL_ERR("gui instance VBO generation")
	glBindBuffer(GL_ARRAY_BUFFER, ctx->gl.instanceVBO);
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
	glVertexAttribDivisor(1, 1);
	GL_ERR("gui instance vertex attrib enable")
	
	// quad uv attribute 
	glVertexAttribPointer(
		2,
		4,
		GL_FLOAT,
		GL_FALSE,
		sizeof(quad),
		(void*)(4 * sizeof(float))
	);
	GL_ERR("gui instance uv attrib")
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);
	GL_ERR("gui instance uv attrib enable")

	// import shader 
	ctx->gl.shd = shaderImport(GUI_VERT_PATH, GUI_FRAG_PATH);
	if(!ctx->gl.shd) return 0;

	// import texture
	ctx->gl.tex = textureImport(GUI_ATLAS_PATH);
	if(!ctx->gl.tex) return 0;

	return 1;
}

void freeGui(void* vCtx) {
	guiContext* ctx = (guiContext*) vCtx;

	// free VBOs and VAO
	glDeleteBuffers(1, &ctx->gl.quadVBO);
	glDeleteBuffers(1, &ctx->gl.instanceVBO);
	glDeleteVertexArrays(1, &ctx->gl.quadVAO);

	// free shader
	shaderFree(ctx->gl.shd);

	// free texture
	textureFree(ctx->gl.tex);

	free(ctx);
}

// forward declarations for GLFW keyboard callbacks
void charCallback(GLFWwindow* win, unsigned int codepoint);
void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods);

guiContext* initGui(window* win) {
	guiContext* ctx = (guiContext*) win->cbak.ctx;
	if(ctx->win == NULL) {
		if(!newGui(ctx)) return NULL;
		ctx->win = win;
		
		// hook into GLFW
		glfwSetWindowUserPointer(win->gl, ctx);

		// attach callbacks
		glfwSetCharCallback(win->gl, charCallback);
		glfwSetKeyCallback(win->gl, keyCallback);

		// set other values
		ctx->inactive = 0;
	}
	
	// reset queue
	ctx->queue.first = ctx->queue.last = 0;

	// reset cursor
	ctx->vPos = 0.0f;

	return ctx;
}

// -- input

// GLFW character callback
void charCallback(GLFWwindow* win, unsigned int codepoint) {
	guiContext* ctx = glfwGetWindowUserPointer(win);
	
	// check if should input 
    if (!ctx || ctx->in.hotId == 0 || ctx->in.keyBufSiz == IN_BUF_SIZ - 1) 
		return;

	// convert to plain ASCII
	ctx->in.keyBuf[ctx->in.keyBufSiz++] = (char)codepoint;
}

// GLFW key callback
void keyCallback(
	GLFWwindow* win, 
	int key, 
	int scancode __attribute__((unused)), 
	int action __attribute__((unused)), 
	int mods __attribute__((unused))
) {
	guiContext* ctx = glfwGetWindowUserPointer(win);

	// go back on backspace
	if(key == GLFW_KEY_BACKSPACE
	&& action == GLFW_PRESS
	&& ctx->in.keyBufSiz > 0) {
		ctx->in.keyBuf[--ctx->in.keyBufSiz] = '\0';
	}
}

void inputGui(window* win) {
	guiContext* ctx = (guiContext*) win->cbak.ctx;

	// get mouse position
	glfwGetCursorPos(win->gl, &ctx->in.xCur, &ctx->in.yCur);

	if(ctx->inactive) {
		ctx->in.curPress = ctx->in.curReles = ctx->in.curDown = 0;
		ctx->in.enter = 0;
		return;
	}

	// get mouse down state
    int down = glfwGetMouseButton(win->gl, GLFW_MOUSE_BUTTON_LEFT);

	// get other mouse states
    ctx->in.curPress = (down && !ctx->in.prevCur);
    ctx->in.curReles = (!down && ctx->in.prevCur);
   	ctx->in.curDown = down;

	// update previous mouse state
    ctx->in.prevCur = down;

	// get enter state
	ctx->in.enter = (glfwGetKey(win->gl, GLFW_KEY_ENTER) == GLFW_PRESS);
}
