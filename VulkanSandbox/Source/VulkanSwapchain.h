#ifndef VULKANSWAPCHAIN_H
#define VULKANSWAPCHAIN_H

#include "VulkanDevice.h"
#include <vulkan/vulkan.h>
#include <vector>

class VulkanSwapchain {
public:
    VulkanSwapchain(VulkanDevice* device, VkSurfaceKHR surface);
    ~VulkanSwapchain();
    
    void init(VkExtent2D& extent);
    void recreate(VkExtent2D extent);
    
	void nextImage();
    void transitionColor(VkCommandBuffer cmdBuffer);
	void transitionPresent(VkCommandBuffer cmdBuffer);
	void present(VkSemaphore waitSem);
	
	VkFormat getFormat() const {
		return format;
	}
	
	uint32_t getImageCount() const {
		return imageCount;
	}
	
	const std::vector<VkImageView> getImageViews() const {
		return imageViews;
	}
	
	VkSemaphore getPresentSemaphore() const {
		return presentSemaphore;
	}
	
	uint32_t getCurrentImageIndex() const {
		return currentImage;
	}
    
private:
    VulkanDevice* device;
    VkSurfaceKHR surface;
    VkSwapchainKHR handle;
    VkFormat format;
    VkColorSpaceKHR colorSpace;
    VkExtent2D resolution;
    uint32_t imageCount;
    VkSwapchainCreateInfoKHR createInfo;
    std::vector<VkImage> images;
    std::vector<bool> imagesTransitioned;
    std::vector<VkImageView> imageViews;
	VkSemaphore presentSemaphore;
	uint32_t currentImage;
    
    bool inited;
    void cleanup();
};

#endif
