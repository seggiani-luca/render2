#include "window.h"
#include "../../lib/glad/glad.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

// -- initialization

// OpenGL context pointer
static GLFWwindow* glCtx; 

// was OpenGL loaded (via GLAD)? (done when the first window is created)
static int winInitialized = 0;

// initializes OpenGL window context
int newGl() {
	printf("%-55s", "Initializing OpenGL ...");
	if (!glfwInit()) {
		printf("Failed to initialize GLFW\n");
		return 0;
	}

	// GLFW window hints 
	glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// GLFW OpenGL version hints
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// GLFW platform hints
	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);

	// GLFW double buffering hints
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

	printf("Success\n");
	return 1;
}

// prints OpenGL information
void printGl() {
	// print OpenGL version
	printf("\nLoaded OpenGL version: %s\n", glGetString(GL_VERSION));

	// print platform
	int platform = glfwGetPlatform();
	switch (platform) {
		case GLFW_PLATFORM_WIN32:   printf("win32"); break;
		case GLFW_PLATFORM_COCOA:   printf("cocoa"); break;
		case GLFW_PLATFORM_WAYLAND: printf("wayland"); break;
		case GLFW_PLATFORM_X11:     printf("x11"); break;
		case GLFW_PLATFORM_NULL:    printf("null"); break;
		default:                    printf("unknown"); break;
	}

	// print renderer
	printf(" on %s\n\n", glGetString(GL_RENDERER));
}

// loads OpenGL (done when the first window is created)
int loadGl() {
	printf("%-55s", "Loading OpenGL ...");
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("Failed to initialize GLAD\n");
		return 0;
	}

	printf("Success\n");
	printGl();
	return 1;
}

void updateGl() {
	glfwPollEvents();
}

void freeGl() {
	if(!winInitialized) return;

	printf("%-55s", "Terminating OpenGL ...");
	glfwTerminate();

	printf("Success\n");
}


// -- windoww

window* newWindow(
	int width,
	int height,
	const char* title,
	renderCallback cback
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
		if (!newGl()) {
			free(win);
			return NULL;
		}
	}

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

	glfwMakeContextCurrent(win->gl);

	// load OpenGL if needed
	if(!winInitialized) {
		if (!loadGl()) {
			glfwDestroyWindow(win->gl);
			free(win);
			return NULL;
		}

		winInitialized = 1;
	}

	// setup framebuffer
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(win->gl, &fbWidth, &fbHeight);
	glViewport(0, 0, fbWidth, fbHeight);
	
	// setup depth
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return win;
}

void freeWindow(window* win) {
	// free context if present
	if(win->cbak.ctx) win->cbak.free(win->cbak.ctx);

	glfwDestroyWindow(win->gl);
	free(win);
}

int updateWindow(window* win) {
	glfwMakeContextCurrent(win->gl);

	// signal if should close
	if (glfwWindowShouldClose(win->gl)) return 0; 

	// call callback with given context
	if(win->cbak.fun) win->cbak.fun(win);
	
	// swap buffers
	glfwSwapBuffers(win->gl);
	return 1;
}
