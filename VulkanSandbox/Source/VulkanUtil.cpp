#include "VulkanUtil.h"

VkCommandBuffer beginSingleUseCmdBuffer(VkDevice device, VkCommandPool pool) {
	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandPool = pool;
	allocateInfo.commandBufferCount = 1;
	
	VkCommandBuffer cmdBuffer;
	vkAllocateCommandBuffers(device, &allocateInfo, &cmdBuffer);
	
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	
	vkBeginCommandBuffer(cmdBuffer, &beginInfo);
	return cmdBuffer;
}
void endSingleUseCmdBuffer(VkDevice device, VkQueue queue, VkCommandPool pool,
						   VkCommandBuffer cmdBuffer) {
	vkEndCommandBuffer(cmdBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, pool, 1, &cmdBuffer);
}
