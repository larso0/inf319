#ifndef VULKANBUFFER_H
#define VULKANBUFFER_H

#include <vulkan/vulkan.h>
#include "VulkanDevice.h"

class VulkanBuffer {
public:
	VulkanBuffer(const VulkanDevice& device, VkDeviceSize size,
		VkBufferUsageFlags usage, VkMemoryPropertyFlags requiredMemoryProperties,
		VkMemoryPropertyFlags optimalMemoryProperties = 0);
	~VulkanBuffer();

	void* mapMemory(VkDeviceSize offset, VkDeviceSize size);
	void unmapMemory();
	void transfer(VkDeviceSize offset, VkDeviceSize size, void* data);

	VkBuffer getHandle() const {
		return buffer;
	}

	VkMemoryPropertyFlags getMemoryProperties() const {
		return memoryProperties;
	}

private:
	VkDeviceSize size;
	VkBuffer buffer;
	const VulkanDevice& device;
	VkMemoryPropertyFlags memoryProperties;
	VkDeviceMemory memory;
	VkMappedMemoryRange mappedMemory;
	VulkanBuffer* stagingBuffer;

	void transfer(const VulkanBuffer& from, VkDeviceSize offset,
		VkDeviceSize size);
};

#endif
