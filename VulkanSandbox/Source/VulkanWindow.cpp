#include "VulkanWindow.h"
#include <stdexcept>

using namespace Engine;
using namespace std;

VulkanWindow::VulkanWindow(VulkanContext& context) :
	context(context),
	handle(nullptr),
	surface(VK_NULL_HANDLE),
	width(800),
	height(600)
{
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	handle = glfwCreateWindow(width, height, "VulkanSandbox", nullptr, nullptr);
	if (!handle) {
		throw runtime_error("Failed to create GLFW window.");
	}

	VkResult result = glfwCreateWindowSurface(context.instance, handle, nullptr,
		&surface);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create window surface.");
	}

	context.pickDevice(surface, &physicalDevice, &device, &presentQueueIndex);
}

VulkanWindow::~VulkanWindow() {
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(context.instance, surface, nullptr);
	glfwDestroyWindow(handle);
}

Renderer& VulkanWindow::getRenderer() {
	throw 1;
}
