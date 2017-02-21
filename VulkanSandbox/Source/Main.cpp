#include <iostream>
#include <stdexcept>
#include "Context.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include <sstream>

using namespace std;

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

	auto extensions = getRequiredExtensions();
	instanceInfo.enabledExtensionCount = (uint32_t)extensions.size();
	instanceInfo.ppEnabledExtensionNames = extensions.data();

#ifdef NDEBUG
	instanceInfo.enabledLayerCount = 0;
	instanceInfo.ppEnabledLayerNames = NULL;
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

	VkFormat colorFormat;
	if (n == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
		colorFormat = VK_FORMAT_B8G8R8_UNORM;
	} else {
		colorFormat = surfaceFormats[0].format;
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

void quit() {
	vkFreeCommandBuffers(context.device, context.commandPool, 1,
		&context.drawCmdBuffer);
	vkFreeCommandBuffers(context.device, context.commandPool, 1,
			&context.setupCmdBuffer);
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
	} catch (const exception& e) {
		cerr << e.what() << endl;
		return 1;
	}

	while (!glfwWindowShouldClose(context.window)) {
		glfwWaitEvents();
	}

	quit();
	return 0;
}
