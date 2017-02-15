#include <iostream>
#include <stdexcept>
#include "Context.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include <sstream>

using namespace std;

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
	if (!(context.window = glfwCreateWindow(800, 600, "VulkanSandbox", nullptr, nullptr))) {
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
	const char* validationLayer = "VK_LAYER_LUNARG_standard_validation";
	{
		auto available = findAvailableLayers();
		auto pred = [validationLayer](const VkLayerProperties& l) -> bool {
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
				return;
			}
		}
	}

	throw runtime_error("No suitable physical device found.");
}

void quit() {
	vkDestroySurfaceKHR(context.instance, context.surface, nullptr);
#ifndef NDEBUG
	vkDestroyDebugReportCallback(context.instance, context.debugCallback, nullptr);
#endif
	vkDestroyInstance(context.instance, nullptr);
	glfwDestroyWindow(context.window);
	glfwTerminate();
}

int main(int argc, char** argv) {
	try {
		createWindow();
		createInstance();
		loadExtensions();
#ifndef NDEBUG
		createDebugCallback();
#endif
		createSurface();
		pickPhysicalDevice();
	} catch (const exception& e) {
		cerr << e.what();
		return 1;
	}

	while (!glfwWindowShouldClose(context.window)) {
		glfwWaitEvents();
	}

	quit();
	return 0;
}
