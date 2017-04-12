#ifndef VULKANIMAGE_H
#define VULKANIMAGE_H

#include <vulkan/vulkan.h>
#include "VulkanDevice.h"

class VulkanImage {
public:
	VulkanImage(const VulkanDevice& device, uint32_t w, uint32_t h,
				VkFormat format, VkImageTiling tiling,
				VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
	~VulkanImage();
	
	VkImage getHandle() const {
		return handle;
	}
	
	VkDeviceSize getSize() const {
		return size;
	}
	
	uint32_t getWidth() const {
		return width;
	}
	
	uint32_t getHeight() const {
		return height;
	}
	
	VkFormat getFormat() const {
		return format;
	}
	
	VkImageTiling getTiling() const {
		return tiling;
	}
	
	VkImageLayout getLayout() const {
		return layout;
	}
	
	void recordTransition(VkImageLayout to, VkCommandBuffer cmdBuffer);
	void recordTransfer(VulkanImage& from, VkCommandBuffer cmdBuffer);
	
	void* mapMemory(VkDeviceSize offset, VkDeviceSize size);
	void unmapMemory();
	void transition(VkImageLayout to);
	void transfer(VulkanImage& from);
	void transferTransition(VulkanImage& from, VkImageLayout to);
	
private:
	const VulkanDevice& device;
	VkImage handle;
	VkDeviceSize size;
	uint32_t width, height;
	VkFormat format;
	VkImageTiling tiling;
	VkImageLayout layout;
	VkMemoryPropertyFlags memoryProperties;
	VkDeviceMemory memory;
	VkMappedMemoryRange mappedMemory;
};

#endif
