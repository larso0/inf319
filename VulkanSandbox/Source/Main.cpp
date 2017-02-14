#include <GLFW/glfw3.h>
#include <iostream>

using namespace std;

static GLFWwindow* window;

int main(int argc, char** argv) {
	if (!glfwInit()) {
		cerr << "Error initializing GLFW.\n";
		return 1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	if (!(window = glfwCreateWindow(800, 600, "VulkanSandbox", nullptr, nullptr))) {
		glfwTerminate();
		cerr << "Error creating GLFW window.\n";
		return 2;
	}

	while (!glfwWindowShouldClose(window)) {
		glfwWaitEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
