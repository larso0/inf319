#include "VulkanSwapchain.h"
#include <vector>
#include <stdexcept>

using namespace std;

VulkanSwapchain::VulkanSwapchain(VulkanDevice* device, VkSurfaceKHR surface) :
device(device),
surface(surface),
handle(VK_NULL_HANDLE),
format(VK_FORMAT_B8G8R8_UNORM),
colorSpace(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR),
imageCount(2),
inited(false)
{
}

VulkanSwapchain::~VulkanSwapchain() {
	cleanup();
}

void VulkanSwapchain::init(VkExtent2D& extent) {
    uint32_t n = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device->getPhysicalDevice(), surface, &n, nullptr);
	vector<VkSurfaceFormatKHR> surfaceFormats(n);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device->getPhysicalDevice(), surface, &n,
		surfaceFormats.data());

	if (n == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
		format = VK_FORMAT_B8G8R8_UNORM;
		colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	} else {
		uint32_t i;
		for (i = 0; i < n && surfaceFormats[i].format != VK_FORMAT_B8G8R8A8_UNORM; i++);
		if (i == n) i = 0;
		format = surfaceFormats[i].format;
		colorSpace = surfaceFormats[i].colorSpace;
	}

    VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->getPhysicalDevice(), surface,
		&surfaceCapabilities);

	if (imageCount < surfaceCapabilities.minImageCount) {
		imageCount = surfaceCapabilities.minImageCount;
	} else if (surfaceCapabilities.maxImageCount != 0 &&
		imageCount > surfaceCapabilities.maxImageCount) {
		imageCount = surfaceCapabilities.maxImageCount;
	}

	resolution = surfaceCapabilities.currentExtent;
    if (resolution.width == 0xFFFFFFFF) {
		resolution = extent;
    } else {
		extent = resolution;
	}

	VkSurfaceTransformFlagBitsKHR preTransform =
		surfaceCapabilities.currentTransform;
	if (surfaceCapabilities.supportedTransforms &
		VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(device->getPhysicalDevice(), surface, &n,
		nullptr);
	vector<VkPresentModeKHR> presentModes(n);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device->getPhysicalDevice(), surface, &n,
		presentModes.data());

	VkPresentModeKHR presentationMode = VK_PRESENT_MODE_FIFO_KHR;
	for (uint32_t i = 0; i < n; i++) {
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			presentationMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
	}

	VkSwapchainKHR oldSwapchain = handle;

	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = format;
	createInfo.imageColorSpace = colorSpace;
	createInfo.imageExtent = resolution;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;
	createInfo.preTransform = preTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentationMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = oldSwapchain;

	VkResult result = vkCreateSwapchainKHR(device->getHandle(), &createInfo,
		nullptr, &handle);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create swapchain.");
	}

	if (inited) {
		for (VkImageView i : imageViews) {
			vkDestroyImageView(device->getHandle(), i, nullptr);
		}
		vkDestroySwapchainKHR(device->getHandle(), oldSwapchain, nullptr);
	}

	vkGetSwapchainImagesKHR(device->getHandle(), handle, &n, nullptr);
	images.resize(n);
	vkGetSwapchainImagesKHR(device->getHandle(), handle, &n, images.data());

	imagesTransitioned = vector<bool>(n, false);

	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = format;
	imageViewInfo.components = {
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A
	};
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;

	imageViews.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; i++) {
		imageViewInfo.image = images[i];
		VkResult result = vkCreateImageView(device->getHandle(), &imageViewInfo,
			nullptr, imageViews.data() + i);
		if (result != VK_SUCCESS) {
			throw runtime_error("Unable to create swapchain image view.");
		}
	}

	VkSemaphoreCreateInfo semInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		0, 0
	};
	if (!inited) {
		vkCreateSemaphore(device->getHandle(), &semInfo, nullptr, &presentSemaphore);
	}

	inited = true;
}

void VulkanSwapchain::recreate(VkExtent2D extent) {
	init(extent);
}

void VulkanSwapchain::nextImage() {
	vkAcquireNextImageKHR(device->getHandle(), handle, UINT64_MAX,
						  presentSemaphore, VK_NULL_HANDLE, &currentImage);
}

void VulkanSwapchain::transitionColor(VkCommandBuffer cmdBuffer) {
	VkImageMemoryBarrier layoutTransitionBarrier = {};
	layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	if (imagesTransitioned[currentImage]) {
		layoutTransitionBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		layoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	} else {
		layoutTransitionBarrier.srcAccessMask = 0;
		layoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imagesTransitioned[currentImage] = true;
	}
	layoutTransitionBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
		| VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	layoutTransitionBarrier.newLayout =
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.image = images[currentImage];
	VkImageSubresourceRange resourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1,
		0, 1 };
	layoutTransitionBarrier.subresourceRange = resourceRange;

	vkCmdPipelineBarrier(cmdBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
		0, nullptr, 0, nullptr, 1, &layoutTransitionBarrier);
}

void VulkanSwapchain::transitionPresent(VkCommandBuffer cmdBuffer) {
	VkImageMemoryBarrier prePresentBarrier = {};
	prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
	prePresentBarrier.image = images[currentImage];

	vkCmdPipelineBarrier(cmdBuffer,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
		&prePresentBarrier);
}

void VulkanSwapchain::present(VkSemaphore waitSem) {
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &waitSem;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &handle;
	presentInfo.pImageIndices = &currentImage;
	presentInfo.pResults = nullptr;
	vkQueuePresentKHR(device->getPresentQueue(), &presentInfo);
}

void VulkanSwapchain::cleanup() {
	if (inited) {
		vkDestroySemaphore(device->getHandle(), presentSemaphore, nullptr);
		for (VkImageView i : imageViews) {
			vkDestroyImageView(device->getHandle(), i, nullptr);
		}
		vkDestroySwapchainKHR(device->getHandle(), handle, nullptr);
	}
}
