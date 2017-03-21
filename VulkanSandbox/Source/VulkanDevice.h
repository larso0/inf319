#ifndef VULKANDEVICE_H
#define VULKANDEVICE_H

#include <vulkan/vulkan.h>
#include "VulkanContext.h"
#include <vector>

class VulkanDevice {
public:
	enum class Capability : int {
		None = 0,
		Transfer = 1,
		Graphics = 1 << 1,
		Compute = 1 << 2
	};

	VulkanDevice(const VulkanContext& context, Capability capability,
		VkSurfaceKHR surface = VK_NULL_HANDLE);
	~VulkanDevice();

	void init();

	int32_t findMemoryType(uint32_t desired,
		VkMemoryPropertyFlags properties) const;

	VkPhysicalDevice getPhysicalDevice() const {
		return physicalDevice;
	}

	const VkPhysicalDeviceProperties& getProperties() const {
		return properties;
	}

	const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const {
		return memoryProperties;
	}

	VkDevice getHandle() const {
		return device;
	}

	VkQueue getTransferQueue() const {
		return transferQueue;
	}

	VkQueue getPresentQueue() const {
		return presentQueue;
	}

	VkQueue getComputeQueue() const {
		return computeQueue;
	}

	VkCommandPool getTransferCommandPool() const {
		return transferCommandPool;
	}

	VkCommandPool getPresentCommandPool() const {
		return presentCommandPool;
	}

	VkCommandPool getComputeCommandPool() const {
		return computeCommandPool;
	}

private:
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	VkPhysicalDeviceFeatures features;
	std::vector<VkDeviceQueueCreateInfo> queues;
	VkDeviceCreateInfo deviceCreateInfo;
	VkDevice device;
	int transferIndex, presentIndex, computeIndex;
	VkQueue transferQueue, presentQueue, computeQueue;
	VkCommandPool transferCommandPool, presentCommandPool, computeCommandPool;
};

VulkanDevice::Capability operator|(VulkanDevice::Capability a,
	VulkanDevice::Capability b);
VulkanDevice::Capability operator&(VulkanDevice::Capability a,
	VulkanDevice::Capability b);
VulkanDevice::Capability operator^(VulkanDevice::Capability a,
	VulkanDevice::Capability b);
VulkanDevice::Capability operator~(VulkanDevice::Capability a);

#endif
