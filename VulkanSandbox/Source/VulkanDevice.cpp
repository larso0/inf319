#include "VulkanDevice.h"
#include <stdexcept>
#include <vector>
#include <iostream>

using namespace std;

VulkanDevice::Capability operator|(VulkanDevice::Capability a,
	VulkanDevice::Capability b) {
	return static_cast<VulkanDevice::Capability>(static_cast<int>(a)
		| static_cast<int>(b));
}

VulkanDevice::Capability operator&(VulkanDevice::Capability a,
	VulkanDevice::Capability b) {
	return static_cast<VulkanDevice::Capability>(static_cast<int>(a)
		& static_cast<int>(b));
}

VulkanDevice::Capability operator^(VulkanDevice::Capability a,
	VulkanDevice::Capability b) {
	return static_cast<VulkanDevice::Capability>(static_cast<int>(a)
		^ static_cast<int>(b));
}

VulkanDevice::Capability operator~(VulkanDevice::Capability a) {
	return static_cast<VulkanDevice::Capability>(~static_cast<int>(a));
}

static const char* swapchainExtension = "VK_KHR_swapchain";

VulkanDevice::VulkanDevice(const VulkanContext& context,
	VulkanDevice::Capability capability, VkSurfaceKHR surface) :
	physicalDevice(VK_NULL_HANDLE),
	device(VK_NULL_HANDLE),
	transferIndex(-1),
	presentIndex(-1),
	computeIndex(-1),
	transferQueue(VK_NULL_HANDLE),
	presentQueue(VK_NULL_HANDLE),
	computeQueue(VK_NULL_HANDLE),
	transferCommandPool(VK_NULL_HANDLE),
	presentCommandPool(VK_NULL_HANDLE),
	computeCommandPool(VK_NULL_HANDLE)
{
	const auto& physicalDevices = context.getPhysicalDevices();
	for (VkPhysicalDevice current : context.getPhysicalDevices()) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(current, &properties);

		uint32_t n;
		vkGetPhysicalDeviceQueueFamilyProperties(current, &n, nullptr);
		vector<VkQueueFamilyProperties> queues(n);
		vkGetPhysicalDeviceQueueFamilyProperties(current, &n, queues.data());

		Capability currentCapability = Capability::None;
		for (int i = 0; i < n; i++) {
			VkQueueFlags flags = queues[i].queueFlags;
			if ((capability & Capability::Transfer) != Capability::None
				&& (currentCapability & Capability::Transfer) == Capability::None
				&& (flags & VK_QUEUE_TRANSFER_BIT)) {
				currentCapability = currentCapability | Capability::Transfer;
				transferIndex = i;
			}

			if ((capability & Capability::Graphics) != Capability::None
				&& (currentCapability & Capability::Graphics) == Capability::None
				&& (flags & VK_QUEUE_GRAPHICS_BIT)) {
				VkBool32 supportsPresent;
				vkGetPhysicalDeviceSurfaceSupportKHR(current, i, surface,
					&supportsPresent);
				if (supportsPresent) {
					currentCapability = currentCapability | Capability::Graphics;
					presentIndex = i;
				}
			}

			if ((capability & Capability::Compute) != Capability::None
				&& (currentCapability & Capability::Compute) == Capability::None
				&& (flags & VK_QUEUE_COMPUTE_BIT)) {
				currentCapability = currentCapability | Capability::Compute;
				computeIndex = i;
			}

			if (currentCapability == capability) break;
		}

		if (currentCapability == capability) {
			physicalDevice = current;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw runtime_error("No suitable physical device found.");
	}

	float priority = 1.f;

	if (transferIndex > -1) {
		queues.push_back({
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			(uint32_t)transferIndex,
			1,
			&priority
		});
	}

	if (presentIndex > -1 && presentIndex != transferIndex) {
		queues.push_back({
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			(uint32_t)presentIndex,
			1,
			&priority
		});
	}

	if (computeIndex > -1 && computeIndex != transferIndex
		&& computeIndex != presentIndex) {
		queues.push_back({
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			(uint32_t)computeIndex,
			1,
			&priority
		});
	}

	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = queues.size();
	deviceCreateInfo.pQueueCreateInfos = queues.data();

	features = {};
	if ((capability & Capability::Graphics) != Capability::None) {
		deviceCreateInfo.enabledExtensionCount = 1;
		deviceCreateInfo.ppEnabledExtensionNames = &swapchainExtension;
		features.shaderClipDistance = VK_TRUE;
		features.samplerAnisotropy = VK_TRUE;
	}
	deviceCreateInfo.pEnabledFeatures = &features;
}

VulkanDevice::~VulkanDevice() {
	if (device != VK_NULL_HANDLE) {
		if (transferCommandPool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(device, transferCommandPool, nullptr);
		}
		if (presentCommandPool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(device, presentCommandPool, nullptr);
		}
		if (computeCommandPool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(device, computeCommandPool, nullptr);
		}
		vkDestroyDevice(device, nullptr);
	}
}

void VulkanDevice::init() {
	VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo,
		nullptr, &device);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create logical device.");
	}

	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (transferIndex > -1) {
		vkGetDeviceQueue(device, transferIndex, 0, &transferQueue);
		commandPoolInfo.queueFamilyIndex = (uint32_t)transferIndex;
		result = vkCreateCommandPool(device, &commandPoolInfo, nullptr,
			&transferCommandPool);
		if (result != VK_SUCCESS) {
			throw runtime_error("Failed to create command pool.");
		}
	}

	if (presentIndex > -1) {
		vkGetDeviceQueue(device, presentIndex, 0, &presentQueue);
		commandPoolInfo.queueFamilyIndex = (uint32_t)presentIndex;
		result = vkCreateCommandPool(device, &commandPoolInfo, nullptr,
			&presentCommandPool);
		if (result != VK_SUCCESS) {
			throw runtime_error("Failed to create command pool.");
		}
	}

	if (computeIndex > -1) {
		vkGetDeviceQueue(device, computeIndex, 0, &computeQueue);
		commandPoolInfo.queueFamilyIndex = (uint32_t)computeIndex;
		result = vkCreateCommandPool(device, &commandPoolInfo, nullptr,
			&computeCommandPool);
		if (result != VK_SUCCESS) {
			throw runtime_error("Failed to create command pool.");
		}
	}
}

int32_t VulkanDevice::findMemoryType(uint32_t desired,
	VkMemoryPropertyFlags properties) const {
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		if ((desired & (1 << i))
			&& (memoryProperties.memoryTypes[i].propertyFlags & properties)
				== properties) {
			return i;
		}
	}

	return -1;
}
