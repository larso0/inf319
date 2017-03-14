#ifndef VULKANBUFFERS_H
#define VULKANBUFFERS_H

#include <vulkan/vulkan.h>

struct VulkanBuffer {
	VkBuffer buffer;
	VkDeviceMemory memory;
};

VulkanBuffer createBuffer(VkDevice device,
	const VkPhysicalDeviceMemoryProperties& memoryProperties, VkDeviceSize size,
	void* data, VkBufferUsageFlags usage);

#endif
