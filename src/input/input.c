#include "input.h"
#include <GLFW/glfw3.h>

// input key name table
const inputKeyName keyNames[] = {
    {"space", KEY_SPACE},
    {"apostrophe", KEY_APOSTROPHE},
    {"comma", KEY_COMMA},
    {"minus", KEY_MINUS},
    {"period", KEY_PERIOD},
    {"slash", KEY_SLASH},
    {"0", KEY_0},
    {"1", KEY_1},
    {"2", KEY_2},
    {"3", KEY_3},
    {"4", KEY_4},
    {"5", KEY_5},
    {"6", KEY_6},
    {"7", KEY_7},
    {"8", KEY_8},
    {"9", KEY_9},
    {"semicolon", KEY_SEMICOLON},
    {"equal", KEY_EQUAL},
    {"a", KEY_A},
    {"b", KEY_B},
    {"c", KEY_C},
    {"d", KEY_D},
    {"e", KEY_E},
    {"f", KEY_F},
    {"g", KEY_G},
    {"h", KEY_H},
    {"i", KEY_I},
    {"j", KEY_J},
    {"k", KEY_K},
    {"l", KEY_L},
    {"m", KEY_M},
    {"n", KEY_N},
    {"o", KEY_O},
    {"p", KEY_P},
    {"q", KEY_Q},
    {"r", KEY_R},
    {"s", KEY_S},
    {"t", KEY_T},
    {"u", KEY_U},
    {"v", KEY_V},
    {"w", KEY_W},
    {"x", KEY_X},
    {"y", KEY_Y},
    {"z", KEY_Z},
    {"left bracket", KEY_LEFT_BRACKET},
    {"backslash", KEY_BACKSLASH},
    {"right bracket", KEY_RIGHT_BRACKET},
    {"grave accent", KEY_GRAVE_ACCENT},
    {"world 1", KEY_WORLD_1},
    {"world 2", KEY_WORLD_2},
    {"escape", KEY_ESCAPE},
    {"enter", KEY_ENTER},
    {"tab", KEY_TAB},
    {"backspace", KEY_BACKSPACE},
    {"insert", KEY_INSERT},
    {"delete", KEY_DELETE},
    {"right", KEY_RIGHT},
    {"left", KEY_LEFT},
    {"down", KEY_DOWN},
    {"up", KEY_UP},
    {"page up", KEY_PAGE_UP},
    {"page down", KEY_PAGE_DOWN},
    {"home", KEY_HOME},
    {"end", KEY_END},
    {"caps lock", KEY_CAPS_LOCK},
    {"scroll lock", KEY_SCROLL_LOCK},
    {"num lock", KEY_NUM_LOCK},
    {"print screen", KEY_PRINT_SCREEN},
    {"pause", KEY_PAUSE},
    {"f1", KEY_F1},
    {"f2", KEY_F2},
    {"f3", KEY_F3},
    {"f4", KEY_F4},
    {"f5", KEY_F5},
    {"f6", KEY_F6},
    {"f7", KEY_F7},
    {"f8", KEY_F8},
    {"f9", KEY_F9},
    {"f10", KEY_F10},
    {"f11", KEY_F11},
    {"f12", KEY_F12},
    {"f13", KEY_F13},
    {"f14", KEY_F14},
    {"f15", KEY_F15},
    {"f16", KEY_F16},
    {"f17", KEY_F17},
    {"f18", KEY_F18},
    {"f19", KEY_F19},
    {"f20", KEY_F20},
    {"f21", KEY_F21},
    {"f22", KEY_F22},
    {"f23", KEY_F23},
    {"f24", KEY_F24},
    {"f25", KEY_F25},
    {"kp 0", KEY_KP_0},
    {"kp 1", KEY_KP_1},
    {"kp 2", KEY_KP_2},
    {"kp 3", KEY_KP_3},
    {"kp 4", KEY_KP_4},
    {"kp 5", KEY_KP_5},
    {"kp 6", KEY_KP_6},
    {"kp 7", KEY_KP_7},
    {"kp 8", KEY_KP_8},
    {"kp 9", KEY_KP_9},
    {"kp decimal", KEY_KP_DECIMAL},
    {"kp divide", KEY_KP_DIVIDE},
    {"kp multiply", KEY_KP_MULTIPLY},
    {"kp subtract", KEY_KP_SUBTRACT},
    {"kp add", KEY_KP_ADD},
    {"kp enter", KEY_KP_ENTER},
    {"kp equal", KEY_KP_EQUAL},
    {"left shift", KEY_LEFT_SHIFT},
    {"left control", KEY_LEFT_CONTROL},
    {"left alt", KEY_LEFT_ALT},
    {"left super", KEY_LEFT_SUPER},
    {"right shift", KEY_RIGHT_SHIFT},
    {"right control", KEY_RIGHT_CONTROL},
    {"right alt", KEY_RIGHT_ALT},
    {"right super", KEY_RIGHT_SUPER},
    {"menu", KEY_MENU},
};

// input mouse button table
const inputKeyName mouseButtonNames[] = {
    {"button 1", MOUSE_BUTTON_1},
    {"button 2", MOUSE_BUTTON_2},
    {"button 3", MOUSE_BUTTON_3},
    {"button 4", MOUSE_BUTTON_4},
    {"button 5", MOUSE_BUTTON_5},
    {"button 6", MOUSE_BUTTON_6},
    {"button 7", MOUSE_BUTTON_7},
    {"button 8", MOUSE_BUTTON_8},
    {"left mouse", MOUSE_BUTTON_LEFT},
    {"right mouse", MOUSE_BUTTON_RIGHT},
    {"middle mouse", MOUSE_BUTTON_MIDDLE},
};

// local input state
inputState localState = {0};

void attachInput(GLFWwindow* gl) {
	glfwSetKeyCallback(gl, inputKeyCallback);
	glfwSetScrollCallback(gl, inputScrollCallback);
	glfwSetCursorPosCallback(gl, inputMouseCallback);
	glfwSetMouseButtonCallback(gl, inputMouseButtonCallback);
}

inputState* inputGet() {
	return &localState;
}

void inputReset() {
	for(int i = 0; i < GLFW_MOUSE_BUTTON_LAST + 1; i++) {
		keyState* state = &localState.mouse.buttons[i];
		state->reles = state->press = 0;
	}

	for(int i = 0; i < GLFW_KEY_LAST + 1; i++) {
		keyState* state = &localState.keys[i];
		state->reles = state->press = 0;
	}

	localState.mouse.scroll = 0.0f;
}

void inputKeyCallback(
    GLFWwindow* gl __attribute__((unused)),
	int key,
	int scancode __attribute__((unused)),
	int action __attribute__((unused)),
	int mods __attribute__((unused))
) {
	if(key < 0 || key > GLFW_KEY_LAST) return;

	keyState* state = &localState.keys[key];

	if(action == GLFW_PRESS) {
		state->press = 1;
		state->reles = 0;
		state->down = 1;
	}
	if(action == GLFW_RELEASE) {
		state->press = 0;
		state->reles = 1;
		state->down = 0;
	}
}

void inputScrollCallback(
    GLFWwindow* gl __attribute__((unused)),
	double x __attribute__((unused)),
	double y
) {
	localState.mouse.scroll += y;
}

void inputMouseCallback(
    GLFWwindow* gl __attribute__((unused)),
	double x,
	double y
) {
	localState.mouse.x = x;
	localState.mouse.y = y;
}

void inputMouseButtonCallback(
    GLFWwindow* gl __attribute__((unused)),
	int button,
	int action,
	int mods __attribute__((unused))
) {
	if(button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return;

	keyState* state = &localState.mouse.buttons[button];

	if(action == GLFW_PRESS) {
		state->press = 1;
		state->reles = 0;
		state->down = 1;
	}
	if(action == GLFW_RELEASE) {
		state->press = 0;
		state->reles = 1;
		state->down = 0;
	}
}
