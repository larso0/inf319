#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

using namespace std;

void errorCallback(int error, const char* description) {
	cerr << "Error: " << description << endl;
}

int main(int argc, char** argv) {
	if (!glfwInit()) {
		cerr << "Failed to initialize GLFW.\n";
		return 1;
	}
	glfwSetErrorCallback(errorCallback);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

	GLFWwindow* window = glfwCreateWindow(640, 480, "Scratchpad", NULL, NULL);
	if (!window) {
		cerr << "Unable to create window.\n";
		glfwTerminate();
		return 2;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
		cerr << "Could not load OpenGL extensions.\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return 3;
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
