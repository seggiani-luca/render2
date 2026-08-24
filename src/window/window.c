#include "window.h"
#include "../../lib/glad/glad.h"
#include "../data/texture/texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -- initialization

// window counter
static int windows = 0;

// OpenGL context pointer
static GLFWwindow* glCtx;

// was OpenGL loaded (via GLAD)? (done when the first window is created)
static int winInitialized = 0;

// initializes OpenGL window context
int newGl() {
	// GLFW platform hints
	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
	
	// initialize GLFW
	if(!glfwInit()) return 0;

	return 1;
}

// prints OpenGL information
void printGl() {
	// print OpenGL version
	printf("Loaded OpenGL version: %s\n", glGetString(GL_VERSION));

	// print platform
	int platform = glfwGetPlatform();
	switch(platform) {
	case GLFW_PLATFORM_WIN32:   printf("win32"); break;
	case GLFW_PLATFORM_COCOA:   printf("cocoa"); break;
	case GLFW_PLATFORM_WAYLAND: printf("wayland"); break;
	case GLFW_PLATFORM_X11:     printf("x11"); break;
	case GLFW_PLATFORM_NULL:    printf("null"); break;
	default:                    printf("unknown"); break;
	}

	// print renderer
	printf(" on %s\n", glGetString(GL_RENDERER));
}

// loads OpenGL (done when the first window is created)
int loadGl() {
	// load OpenGL via GLAD
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))   return 0;

	// print OpenGL info
	printGl();

	return 1;
}

void updateGl() {
	// poll events
	glfwPollEvents();
}

void freeGl() {
	// only if initialized
	if(!winInitialized) return;

	// terminate OpenGL
	glfwTerminate();

	// warn on not freed windows
	if(windows != 0) printf("Warning! Not all windows freed\n");
}

// -- icons

extern int textureDecode(texture* texture, FILE* file, int* rgba);

// loads a single icon, as a texture
windowIcon* loadIcon(const char* path) {
	// open file
	FILE* file = fopen(path, "rb");
	if(file == NULL) return NULL;
	
	// initialize texture
	texture* tex = malloc(sizeof(texture));
	memset(tex, 0, sizeof(texture));
	
	// load data
	int rgba = 0;
	textureDecode(tex, file, &rgba);
	if(!rgba) {
		free(tex);
		return NULL;
	}

	// setup struct
	windowIcon* ico = malloc(sizeof(windowIcon));
	memset(ico, 0, sizeof(windowIcon));
	ico->height = tex->height;
	ico->width = tex->width;
	ico->pixels = tex->data;

	// cleanup
	free(tex);
	fclose(file);

	return ico;
}

// -- window

// callback for window resizing
void resizeCallback(GLFWwindow* gl, int width, int height) {
	window* win = glfwGetWindowUserPointer(gl);

	// setup framebuffer
	win->fbWidth = width;
	win->fbHeight = height;
}

window* newWindow(
	int width,
	int height,
	const char* title,
	renderCallback cback,
	windowIcon* ico,
	int depth	
) {
	// allocate window data
	window* win = malloc(sizeof(window));
	if(!win) return NULL;
	win->height = height;
	win->width = width;
	win->title = title;
	win->cbak = cback;

	// initalize GLFW if needed
	if(!winInitialized) {
		if(!newGl()) {
			free(win);
			return NULL;
		}
	}

	// GLFW window hints
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	// GLFW OpenGL version hints
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// GLFW double buffering hints
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

	// GLFW depth buffer hints 
	glfwWindowHint(GLFW_DEPTH_BITS, 24);

	// create GLFW window
	win->gl = glfwCreateWindow(
		width,
		height,
		title,
		NULL,
		glCtx
	);
	if(!win->gl) {
		free(win);
		return NULL;
	}

	// update OpenGL context
	if(!glCtx) glCtx = win->gl;

	// make context current
	glfwMakeContextCurrent(win->gl);

	// load OpenGL if needed
	if(!winInitialized) {
		if(!loadGl()) {
			glfwDestroyWindow(win->gl);
			free(win);
			return NULL;
		}

		winInitialized = 1;
	}
	
	// setup framebuffer
	glfwGetFramebufferSize(win->gl, &win->fbWidth, &win->fbHeight);
	
	// install framebuffer callback (timing matters on HiDPI)
	glfwSetWindowUserPointer(win->gl, win);
	glfwSetFramebufferSizeCallback(win->gl, resizeCallback);

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

	// setup icon
	glfwSetWindowIcon(win->gl, 1, ico);
	free(ico);

	// inc. window counter
	windows++;
	return win;
}

void freeWindow(window* win) {
	// free context if present
	if(win->cbak.ctx) win->cbak.free(win->cbak.ctx);

	// destroy window
	glfwDestroyWindow(win->gl);
	free(win);

	// dec. window counter
	windows--;
}

int updateWindow(window* win) {
	glfwMakeContextCurrent(win->gl);
	glViewport(0, 0, win->fbWidth, win->fbHeight);

	// signal if should close
	if(glfwWindowShouldClose(win->gl)) return 0;

	// call callback with given context
	if(win->cbak.fun) win->cbak.fun(win);

	// swap buffers
	glfwSwapBuffers(win->gl);
	return 1;
}

void resizeWindow(window* win, int width, int height) {
	// get scale
	float sx, sy;
	glfwGetWindowContentScale(win->gl, &sx, &sy);

	// set size
	win->width = width;
	win->height = height;
	glfwSetWindowSize(win->gl, width * sx, height * sy);
	
	// setup framebuffer
	glfwGetFramebufferSize(win->gl, &win->fbWidth, &win->fbHeight);
	glViewport(0, 0, win->fbWidth, win->fbHeight);
}

void moveWindow(window* win, int x, int y) {
	glfwSetWindowPos(win->gl, x, y);
}

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
