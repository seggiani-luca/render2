#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// -- initialization

// OpenGL versioning
#define GL_VERSION_MAJOR 3
#define GL_VERSION_MINOR 3

// update OpenGL events
void updateGl();

// terminate OpenGL
void freeGl();

// -- windows

// forward declaration for renderCallback
typedef struct window window;

// callback for window rendering
typedef struct {
	// actual rendering function
	void (*fun)(window* win);

	// context for callback
	void* ctx;
	
	// context freeing function
	void (*free)(void* ctx);
} renderCallback;

// window data
struct window {
	// window dimensions
	int width, height;

	// title of window
	const char* title;

	// rendering callback for window
	renderCallback cbak;

	// GLFW window handle
	GLFWwindow* gl;
};

// creates a new window
window* newWindow(
	int width,
	int height,
	const char* title,
	renderCallback cback
);

// updates a window
int updateWindow(window* win);

// frees a window
void freeWindow(window* win);

#endif
