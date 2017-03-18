#include "VulkanContext.h"
#include "VulkanWindow.h"
#include <stdexcept>
#include <vector>
#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>

using namespace Engine;
using namespace std;

static void glfwErrorCallback(int error, const char* description) {
	cerr << "Error: " << description << endl;
}

VulkanContext::VulkanContext() :
	instance(VK_NULL_HANDLE)
#ifndef NDEBUG
	,debugCallback(VK_NULL_HANDLE),
	vkCreateDebugReportCallback(nullptr),
	vkDestroyDebugReportCallback(nullptr)
#endif
{
	if (!glfwInit()) {
		throw runtime_error("Failed to initialize GLFW.");
	}
	glfwSetErrorCallback(glfwErrorCallback);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	createInstance();
	loadExtensions();
#ifndef NDEBUG
	createDebugCallback();
#endif

	uint32_t n = 0;
	vkEnumeratePhysicalDevices(instance, &n, nullptr);
	physicalDevices.resize(n);
	vkEnumeratePhysicalDevices(instance, &n, physicalDevices.data());
}

VulkanContext::~VulkanContext() {
	for (Window* window : windows) {
		delete window;
	}
#ifndef NDEBUG
	vkDestroyDebugReportCallback(instance, debugCallback, nullptr);
#endif
	vkDestroyInstance(instance, nullptr);
	glfwTerminate();
}

#ifndef NDEBUG
static const char* validationLayer = "VK_LAYER_LUNARG_standard_validation";

static vector<VkLayerProperties> findAvailableLayers() {
	uint32_t count = 0;
	vkEnumerateInstanceLayerProperties(&count, nullptr);
	vector<VkLayerProperties> layers(count);
	vkEnumerateInstanceLayerProperties(&count, layers.data());
	return layers;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(
	VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
	uint64_t object, size_t location, int32_t messageCode,
	const char* pLayerPrefix, const char* pMessage, void* pUserData) {
	cerr << pLayerPrefix << ": " << pMessage << endl;
	return VK_FALSE;
}

void VulkanContext::createDebugCallback() {
	VkDebugReportCallbackCreateInfoEXT callbackInfo;
	callbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	callbackInfo.flags =
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	callbackInfo.pfnCallback = debugReportCallback;
	callbackInfo.pUserData = nullptr;

	VkResult result = vkCreateDebugReportCallback(instance,
		&callbackInfo, nullptr, &debugCallback);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create debug report callback.");
	}
}
#endif

void VulkanContext::loadExtensions() {
#ifndef NDEBUG
	vkCreateDebugReportCallback =
		(PFN_vkCreateDebugReportCallbackEXT)
		vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	vkDestroyDebugReportCallback =
		(PFN_vkDestroyDebugReportCallbackEXT)
		vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
#endif
}

static vector<const char*> getRequiredExtensions() {
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

void VulkanContext::createInstance() {
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

	VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);
	if (result != VK_SUCCESS) {
		throw runtime_error("Unable to create Vulkan instance.");
	}
}

Window& VulkanContext::createWindow(int w, int h, int) {
	VulkanWindow* window = new VulkanWindow(*this);
	windows.push_back(window);
	window->resize(w, h);
	window->init();
	return *window;
}
