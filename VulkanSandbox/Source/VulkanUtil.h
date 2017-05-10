#ifndef VULKANUTIL_H
#define VULKANUTIL_H

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

VkCommandBuffer beginSingleUseCmdBuffer(VkDevice device, VkCommandPool pool);
void endSingleUseCmdBuffer(VkDevice device, VkQueue queue, VkCommandPool pool,
						   VkCommandBuffer cmdBuffer);

std::vector<char> readBinaryFile(const std::string& filename);

#endif
