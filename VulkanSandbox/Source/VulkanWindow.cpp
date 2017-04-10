#include "VulkanWindow.h"
#include "VulkanRenderer.h"
#include <stdexcept>

using namespace Engine;
using namespace std;

VulkanWindow::VulkanWindow(VulkanContext& context) :
	context(context),
	handle(nullptr),
	surface(VK_NULL_HANDLE),
	viewport({ 0, 0, 800, 600, 0, 1 }),
	scissor( { { 0, 0 }, { 800, 600 } }),
	device(nullptr),
	swapchain(nullptr),
	presentCommandBuffer(VK_NULL_HANDLE),
	depthImage(VK_NULL_HANDLE),
	depthImageMemory(VK_NULL_HANDLE),
	depthImageView(VK_NULL_HANDLE),
	renderPass(VK_NULL_HANDLE),
	mouse({false, glm::vec2(), glm::vec2()}),
	open(false),
	renderer(nullptr)
{
}

VulkanWindow::~VulkanWindow() {
	if (renderer) delete renderer;
	close();
}

void VulkanWindow::init() {
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

	VkResult result = glfwCreateWindowSurface(context.getInstance(), handle, nullptr,
		&surface);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create window surface.");
	}

	device = new VulkanDevice(context,
		VulkanDevice::Capability::Graphics | VulkanDevice::Capability::Transfer,
		surface);
	device->init();

	swapchain = new VulkanSwapchain(device, surface);
	VkExtent2D extent = { getWidth(), getHeight() };
	swapchain->init(extent);
	viewport.width = extent.width;
	viewport.height = extent.height;
	scissor.extent = extent;

	VkCommandBufferAllocateInfo commandBufferInfo = {};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferInfo.commandPool = device->getPresentCommandPool();
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferInfo.commandBufferCount = 1;

	result = vkAllocateCommandBuffers(device->getHandle(), &commandBufferInfo,
		&presentCommandBuffer);

	createDepthBuffer();
	createRenderPass();
	createFramebuffers();

	open = true;
}

void VulkanWindow::close() {
	if (!open) return;
	for (VkFramebuffer b : framebuffers) {
		vkDestroyFramebuffer(device->getHandle(), b, nullptr);
	}
	vkDestroyRenderPass(device->getHandle(), renderPass, nullptr);
	vkFreeMemory(device->getHandle(), depthImageMemory, nullptr);
	vkDestroyImageView(device->getHandle(), depthImageView, nullptr);
	vkDestroyImage(device->getHandle(), depthImage, nullptr);
	delete swapchain;
	delete device;
	vkDestroySurfaceKHR(context.getInstance(), surface, nullptr);
	glfwDestroyWindow(handle);
}

void VulkanWindow::resize(uint32_t w, uint32_t h){
	viewport.width = w;
	viewport.height = h;
	scissor.extent.width = w;
	scissor.extent.height = h;
}

Renderer& VulkanWindow::getRenderer() {
	if (!renderer) renderer = new VulkanRenderer(*this);
	return *renderer;
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

	VkResult result = vkCreateImage(device->getHandle(), &imageCreateInfo, nullptr,
		&depthImage);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create depth image.");
	}

	VkMemoryRequirements memoryRequirements = {};
	vkGetImageMemoryRequirements(device->getHandle(), depthImage, &memoryRequirements);

	VkMemoryAllocateInfo imageAllocateInfo = {};
	imageAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageAllocateInfo.allocationSize = memoryRequirements.size;
	
	imageAllocateInfo.memoryTypeIndex = device->findMemoryType(
		memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	result = vkAllocateMemory(device->getHandle(), &imageAllocateInfo, nullptr,
		&depthImageMemory);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to allocate depth buffer.");
	}


	result = vkBindImageMemory(device->getHandle(), depthImage, depthImageMemory, 0);
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
	vkCreateFence(device->getHandle(), &fenceCreateInfo, nullptr, &submitFence);

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
	result = vkQueueSubmit(device->getPresentQueue(), 1, &submitInfo, submitFence);

	vkWaitForFences(device->getHandle(), 1, &submitFence, VK_TRUE, UINT64_MAX);
	vkDestroyFence(device->getHandle(), submitFence, nullptr);
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

	result = vkCreateImageView(device->getHandle(), &imageViewCreateInfo, nullptr,
		&depthImageView);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create depth image view.");
	}

}

void VulkanWindow::createRenderPass() {
	VkAttachmentDescription passAttachments[2] = {};
	passAttachments[0].format = swapchain->getFormat();
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

	VkResult result = vkCreateRenderPass(device->getHandle(), &renderPassCreateInfo,
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

	uint32_t imageCount = swapchain->getImageCount();
	framebuffers.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; i++) {
		framebufferAttachments[0] = swapchain->getImageViews()[i];
		VkResult result = vkCreateFramebuffer(device->getHandle(), &framebufferCreateInfo,
			nullptr, framebuffers.data() + i);
		if (result != VK_SUCCESS) {
			throw runtime_error("Failed to create framebuffer.");
		}
	}

}

void VulkanWindow::windowSizeCallback(GLFWwindow* window, int width,
	int height) {
	VulkanWindow& vulkanWindow = *((VulkanWindow*) glfwGetWindowUserPointer(
		window));
	vulkanWindow.resize(width, height);
}

void VulkanWindow::keyCallback(GLFWwindow* handle, int key, int, int action,
	int) {
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

void VulkanWindow::mousePositionCallback(GLFWwindow* handle, double x,
	double y) {
	VulkanWindow& window = *((VulkanWindow*) glfwGetWindowUserPointer(
		handle));
	glm::vec2 position(x, y);
	glm::vec2 motion = position - window.mouse.position;
	if (window.mouse.hidden) {
		window.mouse.motion = position - window.mouse.position;
	}
	window.mouse.position = position;
}
