#include "time.h"
#include <GLFW/glfw3.h>

// stored last time
double lastTime = 0;

float deltaTime() {
	double time = glfwGetTime();
	double delta = time - lastTime;
	lastTime = time;

	return delta;
}
