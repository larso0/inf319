#include "VulkanBuffers.h"
#include <stdexcept>
#include <cstring>

using namespace std;

VulkanBuffer createBuffer(VkDevice device,
	const VkPhysicalDeviceMemoryProperties& memoryProperties, VkDeviceSize size,
	void* data, VkBufferUsageFlags usage) {
	VulkanBuffer buffer;

	VkBufferCreateInfo vertexBufferInfo = {};
	vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferInfo.size = size;
	vertexBufferInfo.usage = usage;
	vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(device, &vertexBufferInfo, nullptr,
		&buffer.buffer);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create buffer.");
	}

	VkMemoryRequirements vertexBufferMemoryRequirements = { };
	vkGetBufferMemoryRequirements(device, buffer.buffer,
		&vertexBufferMemoryRequirements);

	VkMemoryAllocateInfo bufferAllocateInfo = { };
	bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	bufferAllocateInfo.allocationSize = vertexBufferMemoryRequirements.size;

	uint32_t vertexMemoryTypeBits = vertexBufferMemoryRequirements
		.memoryTypeBits;
	VkMemoryPropertyFlags vertexDesiredMemoryFlags =
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	for (uint32_t i = 0; i < 32; ++i) {
		VkMemoryType memoryType = memoryProperties.memoryTypes[i];
		if (vertexMemoryTypeBits & 1) {
			if ((memoryType.propertyFlags & vertexDesiredMemoryFlags)
				== vertexDesiredMemoryFlags) {
				bufferAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vertexMemoryTypeBits = vertexMemoryTypeBits >> 1;
	}

	result = vkAllocateMemory(device, &bufferAllocateInfo, nullptr,
		&buffer.memory);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to allocate buffer memory.");
	}

	if (data != nullptr) {
		void* mapped;
		result = vkMapMemory(device, buffer.memory, 0, VK_WHOLE_SIZE, 0,
			&mapped);
		if (result != VK_SUCCESS) {
			throw runtime_error("Failed to map buffer memory.");
		}

		memcpy(mapped, data, size);
		vkUnmapMemory(device, buffer.memory);
	}

	result = vkBindBufferMemory(device, buffer.buffer, buffer.memory, 0);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to bind buffer memory.");
	}

	return buffer;
}
