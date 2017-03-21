#include <stdexcept>
#include <cstring>
#include "VulkanBuffer.h"

using namespace std;

VulkanBuffer::VulkanBuffer(const VulkanDevice& device, VkDeviceSize size,
	VkBufferUsageFlags usage, VkMemoryPropertyFlags requiredMemoryProperties,
	VkMemoryPropertyFlags optimalMemoryProperties) :
size(size),
buffer(VK_NULL_HANDLE),
device(device),
memoryProperties(optimalMemoryProperties),
memory(VK_NULL_HANDLE)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(device.getHandle(), &bufferInfo, nullptr,
		&buffer);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create buffer.");
	}

	VkMemoryRequirements memoryRequirements = {};
	vkGetBufferMemoryRequirements(device.getHandle(), buffer,
		&memoryRequirements);


	int32_t memoryType = -1;
	if (optimalMemoryProperties != 0) {
		memoryType = device.findMemoryType(memoryRequirements.memoryTypeBits,
			optimalMemoryProperties);
	}
	if (memoryType == -1) {
		memoryType = device.findMemoryType(memoryRequirements.memoryTypeBits,
			requiredMemoryProperties);
		memoryProperties = requiredMemoryProperties;
	}
	if (memoryType == -1) {
		throw runtime_error("No suitable memory type.");
	}

	VkMemoryAllocateInfo memoryInfo = { };
	memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryInfo.allocationSize = memoryRequirements.size;
	memoryInfo.memoryTypeIndex = (uint32_t)memoryType;

	result = vkAllocateMemory(device.getHandle(), &memoryInfo, nullptr,
		&memory);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to allocate buffer memory.");
	}

	result = vkBindBufferMemory(device.getHandle(), buffer, memory, 0);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to bind buffer memory.");
	}

	mappedMemory.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedMemory.pNext = nullptr;
	mappedMemory.memory = memory;
}

VulkanBuffer::~VulkanBuffer() {
	vkFreeMemory(device.getHandle(), memory, nullptr);
	vkDestroyBuffer(device.getHandle(), buffer, nullptr);
}

void* VulkanBuffer::mapMemory(VkDeviceSize offset, VkDeviceSize size) {
	void* mapped;
	VkResult result = vkMapMemory(device.getHandle(), memory, offset, size,
		0, &mapped);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to map buffer memory.");
	}
	mappedMemory.offset = offset;
	mappedMemory.size = size;
	return mapped;
}

void VulkanBuffer::unmapMemory() {
	vkFlushMappedMemoryRanges(device.getHandle(), 1, &mappedMemory);
	vkUnmapMemory(device.getHandle(), memory);
}

void VulkanBuffer::transfer(VkDeviceSize offset, VkDeviceSize size,
	void* data) {
	if (memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
		void* mapped = mapMemory(offset, size);
		memcpy(mapped, data, size);
		unmapMemory();
	} else {

		VulkanBuffer stagingBuffer(device, size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
				| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		void* mapped = stagingBuffer.mapMemory(0, VK_WHOLE_SIZE);
		memcpy(mapped, data, size);
		stagingBuffer.unmapMemory();

	    VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = device.getTransferCommandPool();
		allocInfo.commandBufferCount = 1;

	    VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device.getHandle(), &allocInfo,
			&commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = offset;

		if (size == VK_WHOLE_SIZE) {
			copyRegion.size = this->size - offset;
		} else {
			copyRegion.size = size;
		}

		vkCmdCopyBuffer(commandBuffer, stagingBuffer.getHandle(), buffer, 1,
			&copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(device.getTransferQueue(), 1, &submitInfo,
			VK_NULL_HANDLE);
		vkQueueWaitIdle(device.getTransferQueue());

		vkFreeCommandBuffers(device.getHandle(),
			device.getTransferCommandPool(), 1, &commandBuffer);
	}
}
