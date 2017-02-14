#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>

using namespace std;

static GLFWwindow* window;
static VkInstance instance;

void createWindow() {
	if (!glfwInit()) {
		throw runtime_error("Error initializing GLFW.");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	if (!(window = glfwCreateWindow(800, 600, "VulkanSandbox", nullptr, nullptr))) {
		glfwTerminate();
		throw runtime_error("Error creating GLFW window.");
	}
}

void createInstance() {
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = NULL;
	appInfo.pApplicationName = "VulkanSandbox";
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = "VulkanSandbox";
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = NULL;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledExtensionCount = 0;
	instanceInfo.ppEnabledExtensionNames = NULL;
	instanceInfo.enabledLayerCount = 0;
	instanceInfo.ppEnabledLayerNames = NULL;

	VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);
	if (result != VK_SUCCESS) {
		throw runtime_error("Unable to create Vulkan instance.");
	}
}

void quit() {
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}

int main(int argc, char** argv) {
	try {
		createWindow();
		createInstance();
	} catch (const exception& e) {
		cerr << e.what();
		return 1;
	}

	while (!glfwWindowShouldClose(window)) {
		glfwWaitEvents();
	}

	quit();
	return 0;
}
