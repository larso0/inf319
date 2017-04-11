#include "VulkanImage.h"

VulkanImage::VulkanImage(const VulkanDevice& device, uint32_t w, uint32_t h,
						 VkFormat format, VkImageTiling tiling,
						 VkImageUsageFlags usage,
						 VkMemoryPropertyFlags properties) :
	device(device),
	handle(VK_NULL_HANDLE),
	width(w),
	height(h),
	format(format),
	tiling(tiling),
	layout(VK_IMAGE_LAYOUT_UNDEFINED),
	memoryProperties(properties),
	memory(VK_NULL_HANDLE),
	view(VK_NULL_HANDLE)
{
	
}
