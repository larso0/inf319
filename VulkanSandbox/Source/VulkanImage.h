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
	
	void* mapMemory(VkDeviceSize offset, VkDeviceSize size);
	void unmapMemory();
	void transfer(const VulkanImage& from);
	
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
	VkImageView view;
};

#endif
