#include "GLContext.h"
#include "GLWindow.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>

using namespace std;

static void glfwErrorCallback(int error, const char* description) {
	cerr << "Error: " << description << endl;
}

GLContext::GLContext() : window(nullptr) {
	if (!glfwInit()) {
		throw runtime_error("Failed to initialize GLFW.\n");
	}
	glfwSetErrorCallback(glfwErrorCallback);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
}

GLContext::~GLContext() {
	if (window) delete window;
	glfwTerminate();
}

Engine::Window& GLContext::createWindow(int w, int h, int flags) {
	if (!window) {
		window = new GLWindow();
		window->resize(w, h);
		window->init();
	}
	return *window;
}
