#include "VulkanTexture.h"
#include "VulkanUtil.h"
#include <stdexcept>
#include <cstring>

using namespace std;
using namespace Engine;

VulkanTexture::VulkanTexture(const VulkanDevice& device, const Texture* texture) :
	device(device)
{
	switch (texture->getFormat()) {
	case Texture::Format::Grey: format = VK_FORMAT_R8_UNORM; break;
	case Texture::Format::GreyAlpha: format = VK_FORMAT_R8G8_UNORM; break;
	case Texture::Format::RGBA: format = VK_FORMAT_R8G8B8A8_UNORM; break;
	}
	
	VulkanImage* stagingImage = new VulkanImage(
		device,
		texture->getWidth(),
		texture->getHeight(),
		format,
		VK_IMAGE_TILING_LINEAR,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	
	VkDeviceSize size = stagingImage->getSize();
	void* mapped = stagingImage->mapMemory(0, size);
	memcpy(mapped, texture->getPixelData(), size);
	stagingImage->unmapMemory();
	
	image = new VulkanImage(
		device,
		texture->getWidth(),
		texture->getHeight(),
		format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	
	VkCommandBuffer cmdBuffer = beginSingleUseCmdBuffer(
		device.getHandle(), device.getPresentCommandPool());
	
	image->recordTransfer(*stagingImage, cmdBuffer);
	image->recordTransition(
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmdBuffer);
	
	endSingleUseCmdBuffer(
		device.getHandle(),
		device.getPresentQueue(),
		device.getPresentCommandPool(),
		cmdBuffer
	);
	
	delete stagingImage;
	
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image->getHandle();
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	
	VkResult result = 
		vkCreateImageView(device.getHandle(), &viewInfo, nullptr, &view);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create texture image view.");
	}
}

VulkanTexture::~VulkanTexture() {
	vkDestroyImageView(device.getHandle(), view, nullptr);
	delete image;
}
