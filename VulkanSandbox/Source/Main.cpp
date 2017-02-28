#include <iostream>
#include <stdexcept>
#include "Context.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <fstream>
#include <Render/MeshGeneration.h>

using namespace std;
using namespace Render;

static const char* validationLayer = "VK_LAYER_LUNARG_standard_validation";
static const char* swapchainExtension = "VK_KHR_swapchain";

static Context context;

#ifndef NDEBUG
PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallback = nullptr;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback = nullptr;
#endif


void createWindow() {
	if (!glfwInit()) {
		throw runtime_error("Error initializing GLFW.");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	if (!(context.window = glfwCreateWindow(context.width, context.height,
		"VulkanSandbox", nullptr, nullptr))) {
		throw runtime_error("Error creating GLFW window.");
	}
}

#ifndef NDEBUG
vector<VkLayerProperties> findAvailableLayers() {
	uint32_t count = 0;
	vkEnumerateInstanceLayerProperties(&count, nullptr);
	vector<VkLayerProperties> layers(count);
	vkEnumerateInstanceLayerProperties(&count, layers.data());
	return layers;
}
#endif

vector<const char*> getRequiredExtensions() {
    unsigned n = 0;
    const char** es;
    es = glfwGetRequiredInstanceExtensions(&n);
    vector<const char*> requiredExtensions(es, es + n);
#ifndef NDEBUG
    requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
    vkEnumerateInstanceExtensionProperties(nullptr, &n, nullptr);
    vector<VkExtensionProperties> available(n);
    vkEnumerateInstanceExtensionProperties(nullptr, &n, available.data());
    n = 0;
    for (const char* e : requiredExtensions) {
		if (find_if(available.begin(), available.end(),
			[e](const VkExtensionProperties& l) -> bool {
				return strcmp(l.extensionName, e) == 0;
			}) == available.end()) {
			stringstream ss;
			ss << "Required extension \"" << e << "\" is not available.";
			throw runtime_error(ss.str());
		}
    }
    return requiredExtensions;
}

void createInstance() {
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "VulkanSandbox";
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = "VulkanSandbox";
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = nullptr;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();
	instanceInfo.enabledExtensionCount = (uint32_t)extensions.size();
	instanceInfo.ppEnabledExtensionNames = extensions.data();

#ifdef NDEBUG
	instanceInfo.enabledLayerCount = 0;
	instanceInfo.ppEnabledLayerNames = nullptr;
#else
	{
		auto available = findAvailableLayers();
		auto pred = [](const VkLayerProperties& l) -> bool {
			return strcmp(l.layerName, validationLayer) == 0;
		};
		auto result = find_if(available.begin(), available.end(), pred);
		if (result == available.end()) {
			throw runtime_error("Validation layer is not available.");
		}
	}
	instanceInfo.enabledLayerCount = 1;
	instanceInfo.ppEnabledLayerNames = &validationLayer;
#endif

	VkResult result = vkCreateInstance(&instanceInfo, nullptr, &context.instance);
	if (result != VK_SUCCESS) {
		throw runtime_error("Unable to create Vulkan instance.");
	}
}

void loadExtensions() {
#ifndef NDEBUG
	vkCreateDebugReportCallback =
		(PFN_vkCreateDebugReportCallbackEXT)
		vkGetInstanceProcAddr(context.instance, "vkCreateDebugReportCallbackEXT");
	vkDestroyDebugReportCallback =
		(PFN_vkDestroyDebugReportCallbackEXT)
		vkGetInstanceProcAddr(context.instance, "vkDestroyDebugReportCallbackEXT");
#endif
}

#ifndef NDEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(
	VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
	uint64_t object, size_t location, int32_t messageCode,
	const char* pLayerPrefix, const char* pMessage, void* pUserData) {
	cerr << pLayerPrefix << ": " << pMessage << endl;
	return VK_FALSE;
}

void createDebugCallback() {
	VkDebugReportCallbackCreateInfoEXT callbackInfo;
	callbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	callbackInfo.flags =
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	callbackInfo.pfnCallback = debugReportCallback;
	callbackInfo.pUserData = nullptr;

	VkResult result = vkCreateDebugReportCallback(context.instance,
		&callbackInfo, nullptr, &context.debugCallback);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create debug report callback.");
	}
}
#endif

void createSurface() {
	VkResult result = glfwCreateWindowSurface(context.instance, context.window,
		nullptr, &context.surface);
	if (result != VK_SUCCESS) {
		throw runtime_error("Unable to create window surface.");
	}
}

void pickPhysicalDevice() {
	uint32_t n = 0;
	vkEnumeratePhysicalDevices(context.instance, &n, nullptr);
	vector<VkPhysicalDevice> devices(n);
	vkEnumeratePhysicalDevices(context.instance, &n, devices.data());

	for (VkPhysicalDevice d : devices) {
		context.physicalDevice = d;
		vkGetPhysicalDeviceProperties(d, &context.physicalDeviceProperties);
		vkGetPhysicalDeviceQueueFamilyProperties(d, &n, nullptr);
		vector<VkQueueFamilyProperties> queues(n);
		vkGetPhysicalDeviceQueueFamilyProperties(d, &n, queues.data());

		for (int i = 0; i < n; i++) {
			VkBool32 supportPresent;
			vkGetPhysicalDeviceSurfaceSupportKHR(d, i, context.surface,
				&supportPresent);
			if (supportPresent && (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
				context.presentQueueIdx = i;
				return;
			}
		}
	}

	throw runtime_error("No suitable physical device found.");
}

void createLogicalDevice() {
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = context.presentQueueIdx;
	queueCreateInfo.queueCount = 1;
	float queuePriorities[] = { 1.0f };
	queueCreateInfo.pQueuePriorities = queuePriorities;

	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueCreateInfo;
#ifndef NDEBUG
	deviceInfo.enabledLayerCount = 1;
	deviceInfo.ppEnabledLayerNames = &validationLayer;
#endif
	deviceInfo.enabledExtensionCount = 1;
	deviceInfo.ppEnabledExtensionNames = &swapchainExtension;

	VkPhysicalDeviceFeatures features = {};
	features.shaderClipDistance = VK_TRUE;
	deviceInfo.pEnabledFeatures = &features;

	VkResult result = vkCreateDevice(context.physicalDevice, &deviceInfo,
		nullptr, &context.device);
	if (result != VK_SUCCESS) {
		throw runtime_error("Unable to create logical device.");
	}
}

void createSwapchain() {
	uint32_t n = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(context.physicalDevice,
		context.surface, &n, nullptr);
	vector<VkSurfaceFormatKHR> surfaceFormats(n);
	vkGetPhysicalDeviceSurfaceFormatsKHR(context.physicalDevice,
		context.surface, &n, surfaceFormats.data());

	if (n == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
		context.colorFormat = VK_FORMAT_B8G8R8_UNORM;
	} else {
		context.colorFormat = surfaceFormats[0].format;
	}
	VkColorSpaceKHR colorSpace = surfaceFormats[0].colorSpace;

	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.physicalDevice,
		context.surface, &surfaceCapabilities);

	uint32_t desiredImageCount = 2;
	if (desiredImageCount < surfaceCapabilities.minImageCount) {
		desiredImageCount = surfaceCapabilities.minImageCount;
	} else if (surfaceCapabilities.maxImageCount != 0 &&
		desiredImageCount > surfaceCapabilities.maxImageCount) {
		desiredImageCount = surfaceCapabilities.maxImageCount;
	}

	VkExtent2D surfaceResolution =  surfaceCapabilities.currentExtent;
	if( surfaceResolution.width == -1 ) {
	    surfaceResolution.width = context.width;
	    surfaceResolution.height = context.height;
	} else {
	    context.width = surfaceResolution.width;
	    context.height = surfaceResolution.height;
	}

	VkSurfaceTransformFlagBitsKHR preTransform =
		surfaceCapabilities.currentTransform;
	if (surfaceCapabilities.supportedTransforms &
		VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(context.physicalDevice,
		context.surface, &n, nullptr);
	vector<VkPresentModeKHR> presentModes(n);
	vkGetPhysicalDeviceSurfacePresentModesKHR(context.physicalDevice,
		context.surface, &n, presentModes.data());

	VkPresentModeKHR presentationMode = VK_PRESENT_MODE_FIFO_KHR;
	for (uint32_t i = 0; i < n; i++) {
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			presentationMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = context.surface;
	swapchainCreateInfo.minImageCount = desiredImageCount;
	swapchainCreateInfo.imageFormat = context.colorFormat;
	swapchainCreateInfo.imageColorSpace = colorSpace;
	swapchainCreateInfo.imageExtent = surfaceResolution;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.preTransform = preTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentationMode;
	swapchainCreateInfo.clipped = true;

	VkResult result = vkCreateSwapchainKHR(context.device, &swapchainCreateInfo,
		nullptr, &context.swapchain);
	if (result != VK_SUCCESS) {
		throw runtime_error("Unable to create swapchain.");
	}
}

void createCommandPool() {
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = context.presentQueueIdx;

	VkResult result = vkCreateCommandPool(context.device, &commandPoolInfo,
		nullptr, &context.commandPool);
	if (result != VK_SUCCESS) {
		throw runtime_error("Unable to create command pool.");
	}
}

void createCommandBuffers() {
	VkCommandBufferAllocateInfo commandBufferInfo = {};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferInfo.commandPool = context.commandPool;
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferInfo.commandBufferCount = 1;

	VkResult result =
		vkAllocateCommandBuffers(context.device, &commandBufferInfo,
			&context.setupCmdBuffer);
	if (result != VK_SUCCESS) {
		throw runtime_error("Unable to create setup command buffer.");
	}

	result = vkAllocateCommandBuffers(context.device, &commandBufferInfo,
			&context.drawCmdBuffer);
	if (result != VK_SUCCESS) {
		throw runtime_error("Unable to create draw command buffer.");
	}
}

void setupImagesAndCreateImageViews() {
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(context.device, context.swapchain, &imageCount, nullptr);
	context.swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(context.device, context.swapchain, &imageCount,
		context.swapchainImages.data());

	// Move images from VK_IMAGE_LAYOUT_UNDEFINED to
	// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence submitFence;
	vkCreateFence(context.device, &fenceCreateInfo, nullptr, &submitFence);

	vector<bool> transitioned(imageCount, false);
	uint32_t doneCount = 0;

	while (doneCount != imageCount) {

		VkSemaphore presentCompleteSemaphore;
		VkSemaphoreCreateInfo semaphoreCreateInfo = {
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 0, 0 };
		vkCreateSemaphore(context.device, &semaphoreCreateInfo, nullptr,
			&presentCompleteSemaphore);

		uint32_t nextImageIdx;
		vkAcquireNextImageKHR(context.device, context.swapchain, UINT64_MAX,
			presentCompleteSemaphore, VK_NULL_HANDLE, &nextImageIdx);

		if (!transitioned[nextImageIdx]) {

			// start recording out image layout change barrier on our setup
			// command buffer:
			vkBeginCommandBuffer(context.setupCmdBuffer, &beginInfo);

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
				context.swapchainImages[nextImageIdx];
			VkImageSubresourceRange resourceRange = { VK_IMAGE_ASPECT_COLOR_BIT,
				0, 1, 0, 1 };
			layoutTransitionBarrier.subresourceRange = resourceRange;

			vkCmdPipelineBarrier(context.setupCmdBuffer,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
				&layoutTransitionBarrier);

			vkEndCommandBuffer(context.setupCmdBuffer);

			VkPipelineStageFlags waitStageMash[] = {
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			VkSubmitInfo submitInfo = { };
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
			submitInfo.pWaitDstStageMask = waitStageMash;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &context.setupCmdBuffer;
			submitInfo.signalSemaphoreCount = 0;
			submitInfo.pSignalSemaphores = nullptr;
			VkResult result = vkQueueSubmit(context.presentQueue, 1,
				&submitInfo, submitFence);

			vkWaitForFences(context.device, 1, &submitFence, VK_TRUE,
				UINT64_MAX);
			vkResetFences(context.device, 1, &submitFence);

			vkResetCommandBuffer(context.setupCmdBuffer, 0);

			transitioned[nextImageIdx] = true;
			doneCount++;
		}

		vkDestroySemaphore(context.device, presentCompleteSemaphore, nullptr);

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 0;
		presentInfo.pWaitSemaphores = nullptr;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &context.swapchain;
		presentInfo.pImageIndices = &nextImageIdx;
		vkQueuePresentKHR(context.presentQueue, &presentInfo);
	}

	vkDestroyFence(context.device, submitFence, nullptr);

	// Create the image views
	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = context.colorFormat;
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

	context.swapchainImageViews.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; i++) {
		imageViewInfo.image = context.swapchainImages[i];
		VkResult result = vkCreateImageView(context.device, &imageViewInfo,
			nullptr, context.swapchainImageViews.data() + i);
		if (result != VK_SUCCESS) {
			throw runtime_error("Unable to create swapchain image view.");
		}
	}
}

void createDepthBuffer() {
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = VK_FORMAT_D16_UNORM;
	imageCreateInfo.extent = { context.width, context.height, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkResult result = vkCreateImage(context.device, &imageCreateInfo, nullptr,
		&context.depthImage);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create depth image.");
	}

	VkMemoryRequirements memoryRequirements = {};
	vkGetImageMemoryRequirements(context.device, context.depthImage,
		&memoryRequirements);

	VkMemoryAllocateInfo imageAllocateInfo = {};
	imageAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageAllocateInfo.allocationSize = memoryRequirements.size;

	uint32_t memoryTypeBits = memoryRequirements.memoryTypeBits;
	VkMemoryPropertyFlags desiredMemoryFlags =
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	for (uint32_t i = 0; i < 32; ++i) {
		VkMemoryType memoryType = context.memoryProperties.memoryTypes[i];
		if (memoryTypeBits & 1) {
			if ((memoryType.propertyFlags & desiredMemoryFlags)
				== desiredMemoryFlags) {
				imageAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		memoryTypeBits = memoryTypeBits >> 1;
	}

	result = vkAllocateMemory(context.device, &imageAllocateInfo, nullptr,
		&context.depthImageMemory);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to allocate depth buffer.");
	}


	result =
		vkBindImageMemory(context.device, context.depthImage,
			context.depthImageMemory, 0);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to bind depth buffer to image.");
	}

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(context.setupCmdBuffer, &beginInfo);

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
	layoutTransitionBarrier.image = context.depthImage;
	VkImageSubresourceRange resourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1,
		0, 1 };
	layoutTransitionBarrier.subresourceRange = resourceRange;

	vkCmdPipelineBarrier(context.setupCmdBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
		0, nullptr, 0, nullptr, 1, &layoutTransitionBarrier);

	vkEndCommandBuffer(context.setupCmdBuffer);

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence submitFence;
	vkCreateFence(context.device, &fenceCreateInfo, nullptr, &submitFence);

	VkPipelineStageFlags waitStageMask[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo = { };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = waitStageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &context.setupCmdBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;
	result = vkQueueSubmit(context.presentQueue, 1, &submitInfo, submitFence);

	vkWaitForFences(context.device, 1, &submitFence, VK_TRUE, UINT64_MAX);
	vkDestroyFence(context.device, submitFence, nullptr);
	vkResetCommandBuffer(context.setupCmdBuffer, 0);

	VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	VkImageViewCreateInfo imageViewCreateInfo = { };
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = context.depthImage;
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

	result = vkCreateImageView(context.device, &imageViewCreateInfo, nullptr,
		&context.depthImageView);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create depth image view.");
	}
}

void createRenderPass() {
	VkAttachmentDescription passAttachments[2] = {};
	passAttachments[0].format = context.colorFormat;
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

	VkResult result = vkCreateRenderPass(context.device, &renderPassCreateInfo,
		nullptr, &context.renderPass);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create render pass.");
	}
}

void createFramebuffers() {
	VkImageView framebufferAttachments[2];
	framebufferAttachments[1] = context.depthImageView;

	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = context.renderPass;
	framebufferCreateInfo.attachmentCount = 2;
	framebufferCreateInfo.pAttachments = framebufferAttachments;
	framebufferCreateInfo.width = context.width;
	framebufferCreateInfo.height = context.height;
	framebufferCreateInfo.layers = 1;

	uint32_t imageCount = context.swapchainImages.size();
	context.framebuffers.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; i++) {
		framebufferAttachments[0] = context.swapchainImageViews[i];
		VkResult result = vkCreateFramebuffer(context.device,
			&framebufferCreateInfo, nullptr, context.framebuffers.data() + i);
		if (result != VK_SUCCESS) {
			throw runtime_error("Failed to create framebuffer.");
		}
	}
}

void createVertexBuffer() {
	Mesh cube = generateCube();

	VkBufferCreateInfo vertexBufferInfo = {};
	vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferInfo.size = cube.getVertexDataSize();
	vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(context.device, &vertexBufferInfo, nullptr,
		&context.vertexBuffer);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create vertex buffer.");
	}

	VkMemoryRequirements vertexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(context.device, context.vertexBuffer,
		&vertexBufferMemoryRequirements);

	VkMemoryAllocateInfo bufferAllocateInfo = {};
	bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	bufferAllocateInfo.allocationSize = vertexBufferMemoryRequirements.size;

	uint32_t vertexMemoryTypeBits = vertexBufferMemoryRequirements
		.memoryTypeBits;
	VkMemoryPropertyFlags vertexDesiredMemoryFlags =
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	for (uint32_t i = 0; i < 32; ++i) {
		VkMemoryType memoryType = context.memoryProperties.memoryTypes[i];
		if (vertexMemoryTypeBits & 1) {
			if ((memoryType.propertyFlags & vertexDesiredMemoryFlags)
				== vertexDesiredMemoryFlags) {
				bufferAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vertexMemoryTypeBits = vertexMemoryTypeBits >> 1;
	}

	result = vkAllocateMemory(context.device, &bufferAllocateInfo, nullptr,
		&context.vertexBufferMemory);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to allocate vertex buffer memory.");
	}

	void* mapped;
	result = vkMapMemory(context.device, context.vertexBufferMemory, 0,
		VK_WHOLE_SIZE, 0, &mapped);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to map vertex buffer memory.");
	}

	memcpy(mapped, cube.getVertexData(), cube.getVertexDataSize());
	vkUnmapMemory(context.device, context.vertexBufferMemory);

	result = vkBindBufferMemory(context.device, context.vertexBuffer,
		context.vertexBufferMemory, 0);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to bind vertex buffer memory.");
	}
}

vector<char> readFile(const string& filename) {
    ifstream file(filename, ios::ate | ios::binary);

    if (!file.is_open()) {
        throw runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

void createShaderModule(const string& filename, VkShaderModule* dst) {
	vector<char> code = readFile(filename);

	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = code.size();
	info.pCode = (uint32_t *)code.data();

	VkResult result = vkCreateShaderModule(context.device, &info, nullptr, dst);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create shader module.");
	}
}

void createGraphicsPipeline() {
	VkPipelineLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutCreateInfo.setLayoutCount = 0;
	layoutCreateInfo.pSetLayouts = nullptr;
	layoutCreateInfo.pushConstantRangeCount = 0;
	layoutCreateInfo.pPushConstantRanges = nullptr;

	VkResult result = vkCreatePipelineLayout(context.device, &layoutCreateInfo,
		nullptr, &context.pipelineLayout);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create pipeline layout.");
	}

	VkPipelineShaderStageCreateInfo shaderStageCreateInfo[2] = {};
	shaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageCreateInfo[0].module = context.vertexShaderModule;
	shaderStageCreateInfo[0].pName = "main";        // shader entry point function name
	shaderStageCreateInfo[0].pSpecializationInfo = nullptr;

	shaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageCreateInfo[1].module = context.fragmentShaderModule;
	shaderStageCreateInfo[1].pName = "main";        // shader entry point function name
	shaderStageCreateInfo[1].pSpecializationInfo = nullptr;

	VkVertexInputBindingDescription vertexBindingDescription = {};
	vertexBindingDescription.binding = 0;
	vertexBindingDescription.stride = Vertex::Stride;
	vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription vertexAttributeDescritpion = {};
	vertexAttributeDescritpion.location = 0;
	vertexAttributeDescritpion.binding = 0;
	vertexAttributeDescritpion.format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexAttributeDescritpion.offset = 0;

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
	vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexBindingDescription;
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 1;
	vertexInputStateCreateInfo.pVertexAttributeDescriptions = &vertexAttributeDescritpion;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
	inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = context.width;
	viewport.height = context.height;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;

	VkRect2D scissors = {};
	scissors.offset = { 0, 0 };
	scissors.extent = { context.width, context.height };

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissors;

	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_NONE;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.depthBiasEnable = VK_FALSE;
	rasterizationState.depthBiasConstantFactor = 0;
	rasterizationState.depthBiasClamp = 0;
	rasterizationState.depthBiasSlopeFactor = 0;
	rasterizationState.lineWidth = 1;

	VkPipelineMultisampleStateCreateInfo multisampleState = {};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.sampleShadingEnable = VK_FALSE;
	multisampleState.minSampleShading = 0;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = VK_FALSE;
	multisampleState.alphaToOneEnable = VK_FALSE;

	VkStencilOpState noOPStencilState = {};
	noOPStencilState.failOp = VK_STENCIL_OP_KEEP;
	noOPStencilState.passOp = VK_STENCIL_OP_KEEP;
	noOPStencilState.depthFailOp = VK_STENCIL_OP_KEEP;
	noOPStencilState.compareOp = VK_COMPARE_OP_ALWAYS;
	noOPStencilState.compareMask = 0;
	noOPStencilState.writeMask = 0;
	noOPStencilState.reference = 0;

	VkPipelineDepthStencilStateCreateInfo depthState = {};
	depthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthState.depthTestEnable = VK_TRUE;
	depthState.depthWriteEnable = VK_TRUE;
	depthState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthState.depthBoundsTestEnable = VK_FALSE;
	depthState.stencilTestEnable = VK_FALSE;
	depthState.front = noOPStencilState;
	depthState.back = noOPStencilState;
	depthState.minDepthBounds = 0;
	depthState.maxDepthBounds = 0;

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
	colorBlendAttachmentState.blendEnable = VK_FALSE;
	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.colorWriteMask = 0xf;

	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.logicOp = VK_LOGIC_OP_CLEAR;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachmentState;
	colorBlendState.blendConstants[0] = 0.0;
	colorBlendState.blendConstants[1] = 0.0;
	colorBlendState.blendConstants[2] = 0.0;
	colorBlendState.blendConstants[3] = 0.0;

	VkDynamicState dynamicState[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = 2;
	dynamicStateCreateInfo.pDynamicStates = dynamicState;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStageCreateInfo;
	pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
	pipelineCreateInfo.pTessellationState = nullptr;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pDepthStencilState = &depthState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.layout = context.pipelineLayout;
	pipelineCreateInfo.renderPass = context.renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = nullptr;
	pipelineCreateInfo.basePipelineIndex = 0;

	result = vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1,
		&pipelineCreateInfo, nullptr, &context.pipeline);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create pipeline.");
	}
}

void render() {
	VkSemaphore presentCompleteSemaphore, renderingCompleteSemaphore;
	VkSemaphoreCreateInfo semaphoreCreateInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 0, 0 };
	vkCreateSemaphore(context.device, &semaphoreCreateInfo, nullptr,
		&presentCompleteSemaphore);
	vkCreateSemaphore(context.device, &semaphoreCreateInfo, nullptr,
		&renderingCompleteSemaphore);

	uint32_t nextImageIdx;
	vkAcquireNextImageKHR(context.device, context.swapchain, UINT64_MAX,
		presentCompleteSemaphore, VK_NULL_HANDLE, &nextImageIdx);

	VkCommandBufferBeginInfo beginInfo = { };
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(context.drawCmdBuffer, &beginInfo);

	// change image layout from VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	// to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	VkImageMemoryBarrier layoutTransitionBarrier = { };
	layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	layoutTransitionBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	layoutTransitionBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
		| VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	layoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	layoutTransitionBarrier.newLayout =
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.image = context.swapchainImages[nextImageIdx];
	VkImageSubresourceRange resourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1,
		0, 1 };
	layoutTransitionBarrier.subresourceRange = resourceRange;

	vkCmdPipelineBarrier(context.drawCmdBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
		0, nullptr, 0, nullptr, 1, &layoutTransitionBarrier);

	VkClearValue clearValue[] = { { 0.5f, 0.5f, 0.5f, 1.f }, { 1.f, 0.f } };
	VkRenderPassBeginInfo renderPassBeginInfo = { };
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = context.renderPass;
	renderPassBeginInfo.framebuffer = context.framebuffers[nextImageIdx];
	renderPassBeginInfo.renderArea = {0, 0, context.width, context.height};
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValue;
	vkCmdBeginRenderPass(context.drawCmdBuffer, &renderPassBeginInfo,
		VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(context.drawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		context.pipeline);

    VkViewport viewport = { 0, 0, context.width, context.height, 0, 1 };
    vkCmdSetViewport( context.drawCmdBuffer, 0, 1, &viewport );

    VkRect2D scissor = { 0, 0, context.width, context.height };
    vkCmdSetScissor( context.drawCmdBuffer, 0, 1, &scissor);

	VkDeviceSize offsets = {};
	vkCmdBindVertexBuffers(context.drawCmdBuffer, 0, 1,
		&context.vertexBuffer, &offsets);

	vkCmdDraw(context.drawCmdBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(context.drawCmdBuffer);

	VkImageMemoryBarrier prePresentBarrier = { };
	prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
	prePresentBarrier.image = context.swapchainImages[nextImageIdx];

	vkCmdPipelineBarrier(context.drawCmdBuffer,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
		&prePresentBarrier);

	vkEndCommandBuffer(context.drawCmdBuffer);

	VkFence renderFence;
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(context.device, &fenceCreateInfo, nullptr, &renderFence);

	VkPipelineStageFlags waitStageMash =
		{ VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
	submitInfo.pWaitDstStageMask = &waitStageMash;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &context.drawCmdBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderingCompleteSemaphore;
	vkQueueSubmit(context.presentQueue, 1, &submitInfo, renderFence);

	vkWaitForFences(context.device, 1, &renderFence, VK_TRUE, UINT64_MAX);
	vkDestroyFence(context.device, renderFence, nullptr);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderingCompleteSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &context.swapchain;
	presentInfo.pImageIndices = &nextImageIdx;
	presentInfo.pResults = nullptr;
	vkQueuePresentKHR(context.presentQueue, &presentInfo);

	vkDestroySemaphore(context.device, presentCompleteSemaphore, nullptr);
	vkDestroySemaphore(context.device, renderingCompleteSemaphore, nullptr);
}

void quit() {
	vkDestroyPipelineLayout(context.device, context.pipelineLayout, nullptr);
	vkDestroyPipeline(context.device, context.pipeline, nullptr);
	vkDestroyShaderModule(context.device, context.fragmentShaderModule,
		nullptr);
	vkDestroyShaderModule(context.device, context.vertexShaderModule, nullptr);
	vkFreeMemory(context.device, context.vertexBufferMemory, nullptr);
	vkDestroyBuffer(context.device, context.vertexBuffer, nullptr);
	for (VkFramebuffer b : context.framebuffers) {
		vkDestroyFramebuffer(context.device, b, nullptr);
	}
	vkDestroyRenderPass(context.device, context.renderPass, nullptr);
	vkFreeMemory(context.device, context.depthImageMemory, nullptr);
	vkDestroyImageView(context.device, context.depthImageView, nullptr);
	vkDestroyImage(context.device, context.depthImage, nullptr);
	for (VkImageView i : context.swapchainImageViews) {
		vkDestroyImageView(context.device, i, nullptr);
	}
	//Command buffers are freed when command pool is destroyed.
	vkDestroyCommandPool(context.device, context.commandPool, nullptr);
	vkDestroySwapchainKHR(context.device, context.swapchain, nullptr);
	vkDestroyDevice(context.device, nullptr);
	vkDestroySurfaceKHR(context.instance, context.surface, nullptr);
#ifndef NDEBUG
	vkDestroyDebugReportCallback(context.instance, context.debugCallback,
		nullptr);
#endif
	vkDestroyInstance(context.instance, nullptr);
	glfwDestroyWindow(context.window);
	glfwTerminate();
}

int main(int argc, char** argv) {
	try {
		context.width = 800;
		context.height = 600;
		createWindow();
		createInstance();
		loadExtensions();
#ifndef NDEBUG
		createDebugCallback();
#endif
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapchain();

		vkGetDeviceQueue(context.device, context.presentQueueIdx, 0,
			&context.presentQueue);

		createCommandPool();
		createCommandBuffers();
		setupImagesAndCreateImageViews();

		vkGetPhysicalDeviceMemoryProperties(context.physicalDevice,
			&context.memoryProperties);

		createDepthBuffer();
		createRenderPass();
		createFramebuffers();
		createVertexBuffer();
		createShaderModule("Shaders/Simple.vert.spv",
			&context.vertexShaderModule);
		createShaderModule("Shaders/Simple.frag.spv",
			&context.fragmentShaderModule);
		createGraphicsPipeline();
	} catch (const exception& e) {
		cerr << e.what() << endl;
		return 1;
	}

	while (!glfwWindowShouldClose(context.window)) {
		render();
		glfwWaitEvents();
	}

	quit();
	return 0;
}
