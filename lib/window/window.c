#include "window.h"
#include "../../lib/glad/glad.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -- utils

__attribute__((weak))
void* xmalloc(size_t size) {
	void* ptr = malloc(size);

	if(!ptr && size != 0) {
		printf("Memory allocation failed via malloc()\n");
		exit(1);
	}

	return ptr;
}

// current error message
static const char* strError = NULL;

// -- initialization

// open window counter
static int windows = 0;

// OpenGL context pointer, shared across windows
static GLFWwindow* rootContext = NULL;

// was GLFW initialized?
static int glfwInitialized = 0;

// was OpenGL loaded (function hooks obtained via GLAD)?
static int glLoaded = 0;

// initializes OpenGL window context
int winNewGL() {
	// GLFW platform hints
	glfwInitHint(GLFW_PLATFORM, WIN_USE_WAYLAND ? 
		GLFW_PLATFORM_WAYLAND : GLFW_PLATFORM_X11);

	// initialize GLFW
	if(!glfwInit()) return 0;
	glfwInitialized = 1;

	return 1;
}

// frees OpenGL window context
void winFreeGL() {
	// only if initialized
	if(!glfwInitialized) return;

	// terminate OpenGL
	glfwTerminate();

	// reset state
	glfwInitialized = 0;
	glLoaded = 0;
	rootContext = NULL;

	return;
}

// prints OpenGL information
void winPrintGL() {
	// print platform
	int platform = glfwGetPlatform();
	const char* platfString;
	switch(platform) {
		case GLFW_PLATFORM_WIN32:   platfString = "win32";   break;
		case GLFW_PLATFORM_COCOA:   platfString = "cocoa";   break;
		case GLFW_PLATFORM_WAYLAND: platfString = "wayland"; break;
		case GLFW_PLATFORM_X11:     platfString = "x11";     break;
		case GLFW_PLATFORM_NULL:    platfString = "null";    break;
		default:                    platfString = "unknown"; break;
	}

	// print renderer
	printf("Renderer:\t%s on %s\n",
		platfString,
		(const char*)glGetString(GL_RENDERER)
	);

	// print OpenGL version
	printf("OpenGL:\t%s\n",
		(const char*)glGetString(GL_VERSION)
	);
}

// loads OpenGL (done when the first window is created)
int loadGL() {
	// load OpenGL via GLAD
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return 0;
	glLoaded = 1;

	// at this point, print OpenGL info
	winPrintGL();

	return 1;
}

// -- icons

winIcon* winLoadIcon(int height, int width, uint8_t* data) {
	// allocate icon
	winIcon* ico = xmalloc(sizeof(winIcon));
	memset(ico, 0, sizeof(winIcon));

	// setup icon
	ico->height = height;
	ico->width = width;
	ico->pixels = data;

	return ico;
}

// -- window

// callback for window resizing
void winResizeCallback(GLFWwindow* gl, int width, int height) {
	window* win = glfwGetWindowUserPointer(gl);

	// setup framebuffer
	win->fbWidth = width;
	win->fbHeight = height;
}

window* winNew(
	int width,
	int height,
	const char* title,
	winRenderCallback cbak,
	winIcon* ico,
	int depth	
) {
	if(!ico) {
		strError = "Invalid window icon";
		if(cbak.ctx) cbak.free(cbak.ctx);
		return NULL;
	}

	// allocate window data
	window* win = xmalloc(sizeof(window));

	// setup window data
	win->height = height;
	win->width = width;
	win->title = title;
	win->cbak = cbak;

	// initalize GLFW if needed
	if(!glfwInitialized) {
		if(!winNewGL()) {
			strError = "Couldn't initialize OpenGL window context";
			if(cbak.ctx) cbak.free(cbak.ctx);
			free(win);
			free(ico);
			return NULL;
		}
	}

	// GLFW window hints
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	// GLFW OpenGL version hints
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, WIN_GL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, WIN_GL_VERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// GLFW double buffering hints
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

	// GLFW depth buffer hints 
	glfwWindowHint(GLFW_DEPTH_BITS, depth ? 24 : 0);

	// create GLFW window
	win->gl = glfwCreateWindow(
		width,
		height,
		title,
		NULL,
		rootContext
	);
	if(!win->gl) {
		strError = "Couldn't create OpenGL window";
		if(cbak.ctx) cbak.free(cbak.ctx);
		free(win);
		free(ico);
		return NULL;
	}

	// update OpenGL context
	if(!rootContext) rootContext = win->gl;

	// make context current
	glfwMakeContextCurrent(win->gl);

	// load OpenGL if needed
	if(!glLoaded) {
		if(!loadGL()) {
			strError = "Couldn't load OpenGL hooks";
			if(cbak.ctx) cbak.free(cbak.ctx);
			glfwDestroyWindow(win->gl);
			free(win);
			free(ico);
			return NULL;
		}
	}
	
	// setup framebuffer
	glfwGetFramebufferSize(win->gl, &win->fbWidth, &win->fbHeight);
	
	// install framebuffer callback (timing matters on HiDPI)
	glfwSetWindowUserPointer(win->gl, win);
	glfwSetFramebufferSizeCallback(win->gl, winResizeCallback);

	// setup depth
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if(depth) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
	}

	// setup SRGB
	glEnable(GL_FRAMEBUFFER_SRGB);

	// setup icon
	glfwSetWindowIcon(win->gl, 1, ico);
	free(ico->pixels);
	free(ico);

	// inc. window counter
	windows++;

	return win;
}

void winFree(window* win) {
	if(!win) return;

	// error if trying to destroy root window with other windows alive
	if(win->gl == rootContext && windows != 1) {
		printf(
		  "Warning! Trying to destroy root window with other windows alive.\n"
		  "I will leak memory to prevent dangling pointers.\n");
		return;
	}

	// free context if present
	glfwMakeContextCurrent(win->gl);
	if(win->cbak.ctx) win->cbak.free(win->cbak.ctx);

	// destroy window
	glfwDestroyWindow(win->gl);
	free(win);

	// dec. window counter
	windows--;

	// on last window, free GL context
	if(windows == 0) winFreeGL();
}

int winUpdate(window* win) {
	glfwMakeContextCurrent(win->gl);
	glfwPollEvents();
	glViewport(0, 0, win->fbWidth, win->fbHeight);

	// signal if should close
	if(glfwWindowShouldClose(win->gl)) {
		return 0;
	}

	// call callback with given context
	if(win->cbak.fun) win->cbak.fun(win);

	// swap buffers
	glfwSwapBuffers(win->gl);
	return 1;
}

void winResize(window* win, int width, int height) {
	// get scale
	float sx, sy;
	glfwGetWindowContentScale(win->gl, &sx, &sy);

	// set size
	win->width = width;
	win->height = height;
	glfwSetWindowSize(win->gl, width * sx, height * sy);
	
	// setup framebuffer
	glfwGetFramebufferSize(win->gl, &win->fbWidth, &win->fbHeight);
}

void winMove(window* win, int x, int y) {
	glfwSetWindowPos(win->gl, x, y);
}

void winClose(window* win) {
	glfwSetWindowShouldClose(win->gl, GLFW_TRUE);
}

// -- conversions

float winToFbW(window* win, float from) {
	return from * ((float)win->fbWidth  / win->width);
}

float winToFbH(window* win, float from) {
	return from * ((float)win->fbHeight  / win->height);
}

float fbToWinW(window* win, float from) {
	return from / ((float)win->fbWidth  / win->width);
}

float fbToWinH(window* win, float from) {
	return from / ((float)win->fbHeight  / win->height);
}

// -- errors

// reports window errors
const char* winError() {
	const char* prev = strError;
	strError = NULL;
	return prev;
}
