#include "VulkanImage.h"
#include "VulkanUtil.h"
#include <stdexcept>

using namespace std;

VulkanImage::VulkanImage(const VulkanDevice& device, uint32_t w, uint32_t h,
						 VkFormat format, VkImageTiling tiling,
						 VkImageUsageFlags usage,
						 VkMemoryPropertyFlags properties, VkImageLayout initialLayout) :
	device(device),
	handle(VK_NULL_HANDLE),
	width(w),
	height(h),
	format(format),
	tiling(tiling),
	layout(initialLayout),
	memoryProperties(properties),
	memory(VK_NULL_HANDLE)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = layout;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	
	VkResult result = vkCreateImage(device.getHandle(), &imageInfo, nullptr,
									&handle);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create image.");
	}
	
	VkMemoryRequirements requirements;
	vkGetImageMemoryRequirements(device.getHandle(), handle, &requirements);
	
	int32_t memoryType = device.findMemoryType(requirements.memoryTypeBits,
											   properties);
	if (memoryType == -1) {
		throw runtime_error("No suitable memory type.");
	}
	
	VkMemoryAllocateInfo memoryInfo = {};
	memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryInfo.allocationSize = requirements.size;
	memoryInfo.memoryTypeIndex = (uint32_t)memoryType;
	
	result = vkAllocateMemory(device.getHandle(), &memoryInfo, nullptr, &memory);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to allocate image memory.");
	}
	
	result = vkBindImageMemory(device.getHandle(), handle, memory, 0);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to bind image memory.");
	}
	
	mappedMemory.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedMemory.pNext = nullptr;
	mappedMemory.memory = memory;
	size = requirements.size;
}

VulkanImage::~VulkanImage() {
	vkFreeMemory(device.getHandle(), memory, nullptr);
	vkDestroyImage(device.getHandle(), handle, nullptr);
}

void VulkanImage::recordTransition(VkImageLayout to, VkCommandBuffer cmdBuffer) {
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = layout;
	barrier.newLayout = to;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = handle;
	barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;
	
	if (barrier.oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED &&
		barrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	} else if (barrier.oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED &&
			   barrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	} else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			   barrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	} else if (barrier.newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
								VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	} else {
		throw runtime_error("Unsupported image layout transition.");
	}
	
	vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
						 VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr,
						 0, nullptr, 1, &barrier);
}

void VulkanImage::recordTransfer(VulkanImage& from, VkCommandBuffer cmdBuffer) {
	from.recordTransition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, cmdBuffer);
	recordTransition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmdBuffer);
	
	VkImageSubresourceLayers subResource = {};
	subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subResource.baseArrayLayer = 0;
	subResource.mipLevel = 0;
	subResource.layerCount = 1;
	
	VkImageCopy region = {};
	region.srcSubresource = subResource;
	region.dstSubresource = subResource;
	region.srcOffset = {0, 0, 0};
	region.dstOffset = {0, 0, 0};
	region.extent.width = width;
	region.extent.height = height;
	region.extent.depth = 1;
	
	vkCmdCopyImage(cmdBuffer, from.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				   handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void* VulkanImage::mapMemory(VkDeviceSize offset, VkDeviceSize size) {
	void* mapped;
	VkResult result = vkMapMemory(device.getHandle(), memory, offset, size, 0,
								  &mapped);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to map image memory.");
	}
	mappedMemory.offset = offset;
	mappedMemory.size = size;
	return mapped;
}

void VulkanImage::unmapMemory() {
	vkFlushMappedMemoryRanges(device.getHandle(), 1, &mappedMemory);
	vkUnmapMemory(device.getHandle(), memory);
}

void VulkanImage::transition(VkImageLayout to) {
	VkCommandBuffer cmdBuffer =
		beginSingleUseCmdBuffer(device.getHandle(),
								device.getComputeCommandPool());
	
	recordTransition(to, cmdBuffer);
	
	endSingleUseCmdBuffer(device.getHandle(), device.getPresentQueue(),
						  device.getPresentCommandPool(), cmdBuffer);
}

void VulkanImage::transfer(VulkanImage& from) {
	VkCommandBuffer cmdBuffer =
		beginSingleUseCmdBuffer(device.getHandle(),
								device.getComputeCommandPool());
	
	recordTransfer(from, cmdBuffer);
	
	endSingleUseCmdBuffer(device.getHandle(), device.getPresentQueue(),
						  device.getPresentCommandPool(), cmdBuffer);
}

void VulkanImage::transferTransition(VulkanImage& from, VkImageLayout to) {
	VkCommandBuffer cmdBuffer =
		beginSingleUseCmdBuffer(device.getHandle(),
								device.getComputeCommandPool());
	
	recordTransfer(from, cmdBuffer);
	recordTransition(to, cmdBuffer);
	
	endSingleUseCmdBuffer(device.getHandle(), device.getPresentQueue(),
						  device.getPresentCommandPool(), cmdBuffer);
}
