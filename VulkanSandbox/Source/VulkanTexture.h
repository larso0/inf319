#ifndef VULKANTEXTURE_H
#define VULKANTEXTURE_H

#include "VulkanImage.h"
#include "VulkanDevice.h"
#include <Engine/Texture.h>

class VulkanTexture {
public:
	VulkanTexture(const VulkanDevice& device, const Engine::Texture* texture);
	~VulkanTexture();
	
	VkFormat getFormat() const {
		return format;
	}
	
	VulkanImage* getImage() {
		return image;
	}
	
	const VulkanImage* getImage() const {
		return image;
	}
	
	VkImageView getImageView() const {
		return view;
	}
	
private:
	const VulkanDevice& device;
	VkFormat format;
	VulkanImage* image;
	VkImageView view;
};

#endif
