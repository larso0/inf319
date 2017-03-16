#include "VulkanWindow.h"
#include <stdexcept>

using namespace Engine;
using namespace std;

static void windowSizeCallback(GLFWwindow* window, int width, int height) {
	VulkanWindow& vulkanWindow = *((VulkanWindow*) glfwGetWindowUserPointer(
		window));
	vulkanWindow.resize(width, height);
}

void keyCallback(GLFWwindow* handle, int key, int, int action, int) {
	VulkanWindow& window = *((VulkanWindow*) glfwGetWindowUserPointer(
		handle));
	if (action != GLFW_RELEASE) return;
	switch (key) {
	case GLFW_KEY_ESCAPE:
		glfwSetInputMode(handle, GLFW_CURSOR,
			window.mouse.hidden ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
		window.mouse.hidden = !window.mouse.hidden;
		break;
	default:
		break;
	}
}

void mousePositionCallback(GLFWwindow* handle, double x, double y) {
	VulkanWindow& window = *((VulkanWindow*) glfwGetWindowUserPointer(
		handle));
	glm::vec2 position(x, y);
	glm::vec2 motion = position - window.mouse.position;
	if (window.mouse.hidden) {
		window.mouse.motion = position - window.mouse.position;
	}
	window.mouse.position = position;
}

VulkanWindow::VulkanWindow(VulkanContext& context) :
	context(context),
	handle(nullptr),
	surface(VK_NULL_HANDLE),
	viewport({ 0, 0, 800, 600, 0, 1 }),
	scissor( { { 0, 0 }, { 800, 600 } }),
	colorFormat(VK_FORMAT_B8G8R8_UNORM),
	swapchain(VK_NULL_HANDLE),
	presentCommandPool(VK_NULL_HANDLE),
	presentCommandBuffer(VK_NULL_HANDLE),
	depthImage(VK_NULL_HANDLE),
	depthImageMemory(VK_NULL_HANDLE),
	depthImageView(VK_NULL_HANDLE),
	renderPass(VK_NULL_HANDLE),
	mouse({false, glm::vec2(), glm::vec2(), 0.002f})
{
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	handle = glfwCreateWindow(getWidth(), getHeight(), "VulkanSandbox",
		nullptr, nullptr);
	if (!handle) {
		throw runtime_error("Failed to create GLFW window.");
	}

	glfwSetWindowUserPointer(handle, this);
	glfwSetWindowSizeCallback(handle, windowSizeCallback);
	glfwSetKeyCallback(handle, keyCallback);
	glfwSetCursorPosCallback(handle, mousePositionCallback);
	double x, y;
	glfwGetCursorPos(handle, &x, &y);
	mouse.position = glm::vec2(x, y);

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

	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
	createDepthBuffer();
	createRenderPass();
	createFramebuffers();
}

VulkanWindow::~VulkanWindow() {
	for (VkFramebuffer b : framebuffers) {
		vkDestroyFramebuffer(device, b, nullptr);
	}
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);
	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	for (VkImageView i : presentImageViews) {
		vkDestroyImageView(device, i, nullptr);
	}
	vkDestroyCommandPool(device, presentCommandPool, nullptr);
	vkDestroySwapchainKHR(device, swapchain, nullptr);
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

	VkExtent2D surfaceResolution = surfaceCapabilities.currentExtent;
	if( surfaceResolution.width == -1 ) {
	    surfaceResolution.width = getWidth();
	    surfaceResolution.height = getHeight();
	} else {
		resize(surfaceResolution.width, surfaceResolution.height);
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

void VulkanWindow::createDepthBuffer() {
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = VK_FORMAT_D16_UNORM;
	imageCreateInfo.extent = { getWidth(), getHeight(), 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkResult result = vkCreateImage(device, &imageCreateInfo, nullptr,
		&depthImage);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create depth image.");
	}

	VkMemoryRequirements memoryRequirements = {};
	vkGetImageMemoryRequirements(device, depthImage, &memoryRequirements);

	VkMemoryAllocateInfo imageAllocateInfo = {};
	imageAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageAllocateInfo.allocationSize = memoryRequirements.size;

	uint32_t memoryTypeBits = memoryRequirements.memoryTypeBits;
	VkMemoryPropertyFlags desiredMemoryFlags =
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	for (uint32_t i = 0; i < 32; ++i) {
		VkMemoryType memoryType = memoryProperties.memoryTypes[i];
		if (memoryTypeBits & 1) {
			if ((memoryType.propertyFlags & desiredMemoryFlags)
				== desiredMemoryFlags) {
				imageAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		memoryTypeBits = memoryTypeBits >> 1;
	}

	result = vkAllocateMemory(device, &imageAllocateInfo, nullptr,
		&depthImageMemory);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to allocate depth buffer.");
	}


	result = vkBindImageMemory(device, depthImage, depthImageMemory, 0);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to bind depth buffer to image.");
	}

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(presentCommandBuffer, &beginInfo);

	VkImageMemoryBarrier layoutTransitionBarrier = {};
	layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	layoutTransitionBarrier.srcAccessMask = 0;
	layoutTransitionBarrier.dstAccessMask =
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	layoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	layoutTransitionBarrier.newLayout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.image = depthImage;
	VkImageSubresourceRange resourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1,
		0, 1 };
	layoutTransitionBarrier.subresourceRange = resourceRange;

	vkCmdPipelineBarrier(presentCommandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
		0, nullptr, 0, nullptr, 1, &layoutTransitionBarrier);

	vkEndCommandBuffer(presentCommandBuffer);

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence submitFence;
	vkCreateFence(device, &fenceCreateInfo, nullptr, &submitFence);

	VkPipelineStageFlags waitStageMask[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo = { };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = waitStageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &presentCommandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;
	result = vkQueueSubmit(presentQueue, 1, &submitInfo, submitFence);

	vkWaitForFences(device, 1, &submitFence, VK_TRUE, UINT64_MAX);
	vkDestroyFence(device, submitFence, nullptr);
	vkResetCommandBuffer(presentCommandBuffer, 0);

	VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	VkImageViewCreateInfo imageViewCreateInfo = { };
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = depthImage;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = imageCreateInfo.format;
	imageViewCreateInfo.components = {
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY
	};
	imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	result = vkCreateImageView(device, &imageViewCreateInfo, nullptr,
		&depthImageView);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create depth image view.");
	}

}

void VulkanWindow::createRenderPass() {
	VkAttachmentDescription passAttachments[2] = {};
	passAttachments[0].format = colorFormat;
	passAttachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	passAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	passAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	passAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	passAttachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	passAttachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	passAttachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	passAttachments[1].format = VK_FORMAT_D16_UNORM;
	passAttachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	passAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	passAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	passAttachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	passAttachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	passAttachments[1].initialLayout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	passAttachments[1].finalLayout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentReference = {};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;
	subpass.pDepthStencilAttachment = &depthAttachmentReference;

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 2;
	renderPassCreateInfo.pAttachments = passAttachments;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;

	VkResult result = vkCreateRenderPass(device, &renderPassCreateInfo,
		nullptr, &renderPass);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create render pass.");
	}

}

void VulkanWindow::createFramebuffers() {
	VkImageView framebufferAttachments[2];
	framebufferAttachments[1] = depthImageView;

	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = 2;
	framebufferCreateInfo.pAttachments = framebufferAttachments;
	framebufferCreateInfo.width = getWidth();
	framebufferCreateInfo.height = getHeight();
	framebufferCreateInfo.layers = 1;

	uint32_t imageCount = presentImages.size();
	framebuffers.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; i++) {
		framebufferAttachments[0] = presentImageViews[i];
		VkResult result = vkCreateFramebuffer(device, &framebufferCreateInfo,
			nullptr, framebuffers.data() + i);
		if (result != VK_SUCCESS) {
			throw runtime_error("Failed to create framebuffer.");
		}
	}

}
