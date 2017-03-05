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
	presentCommandPool(VK_NULL_HANDLE),
	presentCommandBuffer(VK_NULL_HANDLE)
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

	VkCommandBufferAllocateInfo commandBufferInfo = {};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferInfo.commandPool = presentCommandPool;
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferInfo.commandBufferCount = 1;

	result = vkAllocateCommandBuffers(device, &commandBufferInfo,
		&presentCommandBuffer);

	vkGetDeviceQueue(device, presentQueueIndex, 0, &presentQueue);

	setupSwapchainImages();
}

VulkanWindow::~VulkanWindow() {
	for (VkImageView i : presentImageViews) {
		vkDestroyImageView(device, i, nullptr);
	}
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

void VulkanWindow::setupSwapchainImages() {
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
	presentImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount,
		presentImages.data());

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence submitFence;
	vkCreateFence(device, &fenceCreateInfo, nullptr, &submitFence);

	vector<bool> transitioned(imageCount, false);
	uint32_t doneCount = 0;

	while (doneCount != imageCount) {
		VkSemaphore presentCompleteSemaphore;
		VkSemaphoreCreateInfo semaphoreCreateInfo = {
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 0, 0 };
		vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr,
			&presentCompleteSemaphore);

		uint32_t nextImageIdx;
		vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
			presentCompleteSemaphore, VK_NULL_HANDLE, &nextImageIdx);

		if (!transitioned[nextImageIdx]) {
			vkBeginCommandBuffer(presentCommandBuffer, &beginInfo);

			VkImageMemoryBarrier layoutTransitionBarrier = {};
			layoutTransitionBarrier.sType =
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			layoutTransitionBarrier.srcAccessMask = 0;
			layoutTransitionBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			layoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			layoutTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			layoutTransitionBarrier.srcQueueFamilyIndex =
				VK_QUEUE_FAMILY_IGNORED;
			layoutTransitionBarrier.dstQueueFamilyIndex =
				VK_QUEUE_FAMILY_IGNORED;
			layoutTransitionBarrier.image =
				presentImages[nextImageIdx];
			VkImageSubresourceRange resourceRange = { VK_IMAGE_ASPECT_COLOR_BIT,
				0, 1, 0, 1 };
			layoutTransitionBarrier.subresourceRange = resourceRange;

			vkCmdPipelineBarrier(presentCommandBuffer,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
				&layoutTransitionBarrier);

			vkEndCommandBuffer(presentCommandBuffer);

			VkPipelineStageFlags waitStageMash[] = {
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			VkSubmitInfo submitInfo = { };
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
			submitInfo.pWaitDstStageMask = waitStageMash;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &presentCommandBuffer;
			submitInfo.signalSemaphoreCount = 0;
			submitInfo.pSignalSemaphores = nullptr;
			VkResult result = vkQueueSubmit(presentQueue, 1,
				&submitInfo, submitFence);

			vkWaitForFences(device, 1, &submitFence, VK_TRUE,
				UINT64_MAX);
			vkResetFences(device, 1, &submitFence);

			vkResetCommandBuffer(presentCommandBuffer, 0);

			transitioned[nextImageIdx] = true;
			doneCount++;
		}

		vkDestroySemaphore(device, presentCompleteSemaphore, nullptr);

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 0;
		presentInfo.pWaitSemaphores = nullptr;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &nextImageIdx;
		vkQueuePresentKHR(presentQueue, &presentInfo);
	}

	vkDestroyFence(device, submitFence, nullptr);

	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = colorFormat;
	imageViewInfo.components = {
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A
	};
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;

	presentImageViews.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; i++) {
		imageViewInfo.image = presentImages[i];
		VkResult result = vkCreateImageView(device, &imageViewInfo,
			nullptr, presentImageViews.data() + i);
		if (result != VK_SUCCESS) {
			throw runtime_error("Unable to create swapchain image view.");
		}
	}
}
