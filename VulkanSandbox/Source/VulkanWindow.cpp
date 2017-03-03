#include "VulkanWindow.h"
#include <stdexcept>

using namespace Engine;
using namespace std;

VulkanWindow::VulkanWindow(VulkanContext& context) :
	context(context),
	handle(nullptr),
	surface(VK_NULL_HANDLE),
	width(800),
	height(600),
	colorFormat(VK_FORMAT_B8G8R8_UNORM),
	swapchain(VK_NULL_HANDLE),
	presentCommandPool(VK_NULL_HANDLE)
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
	createSwapchain();
	createCommandPool();
}

VulkanWindow::~VulkanWindow() {
	vkDestroyCommandPool(device, presentCommandPool, nullptr);
	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(context.instance, surface, nullptr);
	glfwDestroyWindow(handle);
}

Renderer& VulkanWindow::getRenderer() {
	throw 1;
}

void VulkanWindow::createSwapchain() {
	uint32_t n = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &n, nullptr);
	vector<VkSurfaceFormatKHR> surfaceFormats(n);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &n,
		surfaceFormats.data());

	if (n != 1 || surfaceFormats[0].format != VK_FORMAT_UNDEFINED) {
		colorFormat = surfaceFormats[0].format;
	}
	VkColorSpaceKHR colorSpace = surfaceFormats[0].colorSpace;

	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
		&surfaceCapabilities);

	uint32_t desiredImageCount = 2;
	if (desiredImageCount < surfaceCapabilities.minImageCount) {
		desiredImageCount = surfaceCapabilities.minImageCount;
	} else if (surfaceCapabilities.maxImageCount != 0 &&
		desiredImageCount > surfaceCapabilities.maxImageCount) {
		desiredImageCount = surfaceCapabilities.maxImageCount;
	}

	VkExtent2D surfaceResolution =  surfaceCapabilities.currentExtent;
	if( surfaceResolution.width == -1 ) {
	    surfaceResolution.width = width;
	    surfaceResolution.height = height;
	} else {
	    width = surfaceResolution.width;
	    height = surfaceResolution.height;
	}

	VkSurfaceTransformFlagBitsKHR preTransform =
		surfaceCapabilities.currentTransform;
	if (surfaceCapabilities.supportedTransforms &
		VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &n,
		nullptr);
	vector<VkPresentModeKHR> presentModes(n);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &n,
		presentModes.data());

	VkPresentModeKHR presentationMode = VK_PRESENT_MODE_FIFO_KHR;
	for (uint32_t i = 0; i < n; i++) {
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			presentationMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = desiredImageCount;
	swapchainCreateInfo.imageFormat = colorFormat;
	swapchainCreateInfo.imageColorSpace = colorSpace;
	swapchainCreateInfo.imageExtent = surfaceResolution;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.preTransform = preTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentationMode;
	swapchainCreateInfo.clipped = true;

	VkResult result = vkCreateSwapchainKHR(device, &swapchainCreateInfo,
		nullptr, &swapchain);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create swapchain.");
	}
}

void VulkanWindow::createCommandPool() {
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = presentQueueIndex;

	VkResult result = vkCreateCommandPool(device, &commandPoolInfo, nullptr,
		&presentCommandPool);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create command pool.");
	}
}
