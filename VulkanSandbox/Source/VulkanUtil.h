#ifndef VULKANUTIL_H
#define VULKANUTIL_H

#include <vulkan/vulkan.h>

VkCommandBuffer beginSingleUseCmdBuffer(VkDevice device, VkCommandPool pool);
void endSingleUseCmdBuffer(VkDevice device, VkQueue queue, VkCommandPool pool,
						   VkCommandBuffer cmdBuffer);

#endif
