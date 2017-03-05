#include <Engine/MeshGeneration.h>
#include <iostream>
#include <stdexcept>
#include "Context.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <fstream>
#include "VulkanContext.h"
#include "VulkanWindow.h"

using namespace std;
using namespace Engine;

static ContextOld context;

void createDepthBuffer(VulkanWindow& window) {
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = VK_FORMAT_D16_UNORM;
	imageCreateInfo.extent = { window.width, window.height, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkResult result = vkCreateImage(window.device, &imageCreateInfo, nullptr,
		&context.depthImage);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create depth image.");
	}

	VkMemoryRequirements memoryRequirements = {};
	vkGetImageMemoryRequirements(window.device, context.depthImage,
		&memoryRequirements);

	VkMemoryAllocateInfo imageAllocateInfo = {};
	imageAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageAllocateInfo.allocationSize = memoryRequirements.size;

	uint32_t memoryTypeBits = memoryRequirements.memoryTypeBits;
	VkMemoryPropertyFlags desiredMemoryFlags =
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	for (uint32_t i = 0; i < 32; ++i) {
		VkMemoryType memoryType = context.memoryProperties.memoryTypes[i];
		if (memoryTypeBits & 1) {
			if ((memoryType.propertyFlags & desiredMemoryFlags)
				== desiredMemoryFlags) {
				imageAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		memoryTypeBits = memoryTypeBits >> 1;
	}

	result = vkAllocateMemory(window.device, &imageAllocateInfo, nullptr,
		&context.depthImageMemory);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to allocate depth buffer.");
	}


	result =
		vkBindImageMemory(window.device, context.depthImage,
			context.depthImageMemory, 0);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to bind depth buffer to image.");
	}

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(window.presentCommandBuffer, &beginInfo);

	VkImageMemoryBarrier layoutTransitionBarrier = {};
	layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	layoutTransitionBarrier.srcAccessMask = 0;
	layoutTransitionBarrier.dstAccessMask =
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	layoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	layoutTransitionBarrier.newLayout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.image = context.depthImage;
	VkImageSubresourceRange resourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1,
		0, 1 };
	layoutTransitionBarrier.subresourceRange = resourceRange;

	vkCmdPipelineBarrier(window.presentCommandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
		0, nullptr, 0, nullptr, 1, &layoutTransitionBarrier);

	vkEndCommandBuffer(window.presentCommandBuffer);

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence submitFence;
	vkCreateFence(window.device, &fenceCreateInfo, nullptr, &submitFence);

	VkPipelineStageFlags waitStageMask[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo = { };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = waitStageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &window.presentCommandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;
	result = vkQueueSubmit(window.presentQueue, 1, &submitInfo, submitFence);

	vkWaitForFences(window.device, 1, &submitFence, VK_TRUE, UINT64_MAX);
	vkDestroyFence(window.device, submitFence, nullptr);
	vkResetCommandBuffer(window.presentCommandBuffer, 0);

	VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	VkImageViewCreateInfo imageViewCreateInfo = { };
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = context.depthImage;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = imageCreateInfo.format;
	imageViewCreateInfo.components = {
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY
	};
	imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	result = vkCreateImageView(window.device, &imageViewCreateInfo, nullptr,
		&context.depthImageView);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create depth image view.");
	}
}

void createRenderPass(VulkanWindow& window) {
	VkAttachmentDescription passAttachments[2] = {};
	passAttachments[0].format = window.colorFormat;
	passAttachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	passAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	passAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	passAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	passAttachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	passAttachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	passAttachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	passAttachments[1].format = VK_FORMAT_D16_UNORM;
	passAttachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	passAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	passAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	passAttachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	passAttachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	passAttachments[1].initialLayout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	passAttachments[1].finalLayout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentReference = {};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;
	subpass.pDepthStencilAttachment = &depthAttachmentReference;

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 2;
	renderPassCreateInfo.pAttachments = passAttachments;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;

	VkResult result = vkCreateRenderPass(window.device, &renderPassCreateInfo,
		nullptr, &context.renderPass);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create render pass.");
	}
}

void createFramebuffers(VulkanWindow& window) {
	VkImageView framebufferAttachments[2];
	framebufferAttachments[1] = context.depthImageView;

	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = context.renderPass;
	framebufferCreateInfo.attachmentCount = 2;
	framebufferCreateInfo.pAttachments = framebufferAttachments;
	framebufferCreateInfo.width = window.width;
	framebufferCreateInfo.height = window.height;
	framebufferCreateInfo.layers = 1;

	uint32_t imageCount = window.presentImages.size();
	context.framebuffers.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; i++) {
		framebufferAttachments[0] = window.presentImageViews[i];
		VkResult result = vkCreateFramebuffer(window.device,
			&framebufferCreateInfo, nullptr, context.framebuffers.data() + i);
		if (result != VK_SUCCESS) {
			throw runtime_error("Failed to create framebuffer.");
		}
	}
}

void createVertexBuffer(VulkanWindow& window) {
	Mesh cube = generateCube();

	VkBufferCreateInfo vertexBufferInfo = {};
	vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferInfo.size = cube.getVertexDataSize();
	vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(window.device, &vertexBufferInfo, nullptr,
		&context.vertexBuffer);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create vertex buffer.");
	}

	VkMemoryRequirements vertexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(window.device, context.vertexBuffer,
		&vertexBufferMemoryRequirements);

	VkMemoryAllocateInfo bufferAllocateInfo = {};
	bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	bufferAllocateInfo.allocationSize = vertexBufferMemoryRequirements.size;

	uint32_t vertexMemoryTypeBits = vertexBufferMemoryRequirements
		.memoryTypeBits;
	VkMemoryPropertyFlags vertexDesiredMemoryFlags =
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	for (uint32_t i = 0; i < 32; ++i) {
		VkMemoryType memoryType = context.memoryProperties.memoryTypes[i];
		if (vertexMemoryTypeBits & 1) {
			if ((memoryType.propertyFlags & vertexDesiredMemoryFlags)
				== vertexDesiredMemoryFlags) {
				bufferAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vertexMemoryTypeBits = vertexMemoryTypeBits >> 1;
	}

	result = vkAllocateMemory(window.device, &bufferAllocateInfo, nullptr,
		&context.vertexBufferMemory);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to allocate vertex buffer memory.");
	}

	void* mapped;
	result = vkMapMemory(window.device, context.vertexBufferMemory, 0,
		VK_WHOLE_SIZE, 0, &mapped);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to map vertex buffer memory.");
	}

	memcpy(mapped, cube.getVertexData(), cube.getVertexDataSize());
	vkUnmapMemory(window.device, context.vertexBufferMemory);

	result = vkBindBufferMemory(window.device, context.vertexBuffer,
		context.vertexBufferMemory, 0);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to bind vertex buffer memory.");
	}
}

vector<char> readFile(const string& filename) {
    ifstream file(filename, ios::ate | ios::binary);

    if (!file.is_open()) {
        throw runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

void createShaderModule(const string& filename, VkShaderModule* dst,
	VkDevice device) {
	vector<char> code = readFile(filename);

	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = code.size();
	info.pCode = (uint32_t *)code.data();

	VkResult result = vkCreateShaderModule(device, &info, nullptr, dst);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create shader module.");
	}
}

void createGraphicsPipeline(VulkanWindow& window) {
	VkPipelineLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutCreateInfo.setLayoutCount = 0;
	layoutCreateInfo.pSetLayouts = nullptr;
	layoutCreateInfo.pushConstantRangeCount = 0;
	layoutCreateInfo.pPushConstantRanges = nullptr;

	VkResult result = vkCreatePipelineLayout(window.device, &layoutCreateInfo,
		nullptr, &context.pipelineLayout);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create pipeline layout.");
	}

	VkPipelineShaderStageCreateInfo shaderStageCreateInfo[2] = {};
	shaderStageCreateInfo[0].sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageCreateInfo[0].module = context.vertexShaderModule;
	shaderStageCreateInfo[0].pName = "main";
	shaderStageCreateInfo[0].pSpecializationInfo = nullptr;

	shaderStageCreateInfo[1].sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageCreateInfo[1].module = context.fragmentShaderModule;
	shaderStageCreateInfo[1].pName = "main";
	shaderStageCreateInfo[1].pSpecializationInfo = nullptr;

	VkVertexInputBindingDescription vertexBindingDescription = {};
	vertexBindingDescription.binding = 0;
	vertexBindingDescription.stride = Vertex::Stride;
	vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription vertexAttributeDescritpion = {};
	vertexAttributeDescritpion.location = 0;
	vertexAttributeDescritpion.binding = 0;
	vertexAttributeDescritpion.format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexAttributeDescritpion.offset = 0;

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
	vertexInputStateCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputStateCreateInfo.pVertexBindingDescriptions =
		&vertexBindingDescription;
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 1;
	vertexInputStateCreateInfo.pVertexAttributeDescriptions =
		&vertexAttributeDescritpion;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
	inputAssemblyStateCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = window.width;
	viewport.height = window.height;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;

	VkRect2D scissors = {};
	scissors.offset = { 0, 0 };
	scissors.extent = { window.width, window.height };

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissors;

	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType =
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_NONE;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.depthBiasEnable = VK_FALSE;
	rasterizationState.depthBiasConstantFactor = 0;
	rasterizationState.depthBiasClamp = 0;
	rasterizationState.depthBiasSlopeFactor = 0;
	rasterizationState.lineWidth = 1;

	VkPipelineMultisampleStateCreateInfo multisampleState = {};
	multisampleState.sType =
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.sampleShadingEnable = VK_FALSE;
	multisampleState.minSampleShading = 0;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = VK_FALSE;
	multisampleState.alphaToOneEnable = VK_FALSE;

	VkStencilOpState noOPStencilState = {};
	noOPStencilState.failOp = VK_STENCIL_OP_KEEP;
	noOPStencilState.passOp = VK_STENCIL_OP_KEEP;
	noOPStencilState.depthFailOp = VK_STENCIL_OP_KEEP;
	noOPStencilState.compareOp = VK_COMPARE_OP_ALWAYS;
	noOPStencilState.compareMask = 0;
	noOPStencilState.writeMask = 0;
	noOPStencilState.reference = 0;

	VkPipelineDepthStencilStateCreateInfo depthState = {};
	depthState.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthState.depthTestEnable = VK_TRUE;
	depthState.depthWriteEnable = VK_TRUE;
	depthState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthState.depthBoundsTestEnable = VK_FALSE;
	depthState.stencilTestEnable = VK_FALSE;
	depthState.front = noOPStencilState;
	depthState.back = noOPStencilState;
	depthState.minDepthBounds = 0;
	depthState.maxDepthBounds = 0;

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
	colorBlendAttachmentState.blendEnable = VK_FALSE;
	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
	colorBlendAttachmentState.dstColorBlendFactor =
		VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.colorWriteMask = 0xf;

	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType =
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.logicOp = VK_LOGIC_OP_CLEAR;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachmentState;
	colorBlendState.blendConstants[0] = 0.0;
	colorBlendState.blendConstants[1] = 0.0;
	colorBlendState.blendConstants[2] = 0.0;
	colorBlendState.blendConstants[3] = 0.0;

	VkDynamicState dynamicState[2] = { VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = 2;
	dynamicStateCreateInfo.pDynamicStates = dynamicState;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStageCreateInfo;
	pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
	pipelineCreateInfo.pTessellationState = nullptr;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pDepthStencilState = &depthState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.layout = context.pipelineLayout;
	pipelineCreateInfo.renderPass = context.renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = nullptr;
	pipelineCreateInfo.basePipelineIndex = 0;

	result = vkCreateGraphicsPipelines(window.device, VK_NULL_HANDLE, 1,
		&pipelineCreateInfo, nullptr, &context.pipeline);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create pipeline.");
	}
}

void render(VulkanWindow& window) {
	VkSemaphore presentCompleteSemaphore, renderingCompleteSemaphore;
	VkSemaphoreCreateInfo semaphoreCreateInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 0, 0 };
	vkCreateSemaphore(window.device, &semaphoreCreateInfo, nullptr,
		&presentCompleteSemaphore);
	vkCreateSemaphore(window.device, &semaphoreCreateInfo, nullptr,
		&renderingCompleteSemaphore);

	uint32_t nextImageIdx;
	vkAcquireNextImageKHR(window.device, window.swapchain, UINT64_MAX,
		presentCompleteSemaphore, VK_NULL_HANDLE, &nextImageIdx);

	VkCommandBufferBeginInfo beginInfo = { };
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(window.presentCommandBuffer, &beginInfo);

	// change image layout from VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	// to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	VkImageMemoryBarrier layoutTransitionBarrier = { };
	layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	layoutTransitionBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	layoutTransitionBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
		| VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	layoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	layoutTransitionBarrier.newLayout =
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.image = window.presentImages[nextImageIdx];
	VkImageSubresourceRange resourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1,
		0, 1 };
	layoutTransitionBarrier.subresourceRange = resourceRange;

	vkCmdPipelineBarrier(window.presentCommandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
		0, nullptr, 0, nullptr, 1, &layoutTransitionBarrier);

	VkClearValue clearValue[] = { { 0.5f, 0.5f, 0.5f, 1.f }, { 1.f, 0.f } };
	VkRenderPassBeginInfo renderPassBeginInfo = { };
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = context.renderPass;
	renderPassBeginInfo.framebuffer = context.framebuffers[nextImageIdx];
	renderPassBeginInfo.renderArea = {0, 0, window.width, window.height};
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValue;
	vkCmdBeginRenderPass(window.presentCommandBuffer, &renderPassBeginInfo,
		VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(window.presentCommandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS, context.pipeline);

	VkViewport viewport = { 0, 0, (float) window.width, (float) window.height,
		0, 1 };
    vkCmdSetViewport(window.presentCommandBuffer, 0, 1, &viewport);

    VkRect2D scissor = { 0, 0, window.width, window.height };
    vkCmdSetScissor(window.presentCommandBuffer, 0, 1, &scissor);

	VkDeviceSize offsets = {};
	vkCmdBindVertexBuffers(window.presentCommandBuffer, 0, 1,
		&context.vertexBuffer, &offsets);

	vkCmdDraw(window.presentCommandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(window.presentCommandBuffer);

	VkImageMemoryBarrier prePresentBarrier = { };
	prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
	prePresentBarrier.image = window.presentImages[nextImageIdx];

	vkCmdPipelineBarrier(window.presentCommandBuffer,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
		&prePresentBarrier);

	vkEndCommandBuffer(window.presentCommandBuffer);

	VkFence renderFence;
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(window.device, &fenceCreateInfo, nullptr, &renderFence);

	VkPipelineStageFlags waitStageMash =
		{ VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
	submitInfo.pWaitDstStageMask = &waitStageMash;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &window.presentCommandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderingCompleteSemaphore;
	vkQueueSubmit(window.presentQueue, 1, &submitInfo, renderFence);

	vkWaitForFences(window.device, 1, &renderFence, VK_TRUE, UINT64_MAX);
	vkDestroyFence(window.device, renderFence, nullptr);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderingCompleteSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &window.swapchain;
	presentInfo.pImageIndices = &nextImageIdx;
	presentInfo.pResults = nullptr;
	vkQueuePresentKHR(window.presentQueue, &presentInfo);

	vkDestroySemaphore(window.device, presentCompleteSemaphore, nullptr);
	vkDestroySemaphore(window.device, renderingCompleteSemaphore, nullptr);
}

void quit(VulkanWindow& window) {
	vkDestroyPipelineLayout(window.device, context.pipelineLayout, nullptr);
	vkDestroyPipeline(window.device, context.pipeline, nullptr);
	vkDestroyShaderModule(window.device, context.fragmentShaderModule,
		nullptr);
	vkDestroyShaderModule(window.device, context.vertexShaderModule, nullptr);
	vkFreeMemory(window.device, context.vertexBufferMemory, nullptr);
	vkDestroyBuffer(window.device, context.vertexBuffer, nullptr);
	for (VkFramebuffer b : context.framebuffers) {
		vkDestroyFramebuffer(window.device, b, nullptr);
	}
	vkDestroyRenderPass(window.device, context.renderPass, nullptr);
	vkFreeMemory(window.device, context.depthImageMemory, nullptr);
	vkDestroyImageView(window.device, context.depthImageView, nullptr);
	vkDestroyImage(window.device, context.depthImage, nullptr);
}

int main(int argc, char** argv) {
	try {
		VulkanContext vkContext;
		VulkanWindow window(vkContext);

		vkGetPhysicalDeviceMemoryProperties(window.physicalDevice,
			&context.memoryProperties);

		createDepthBuffer(window);
		createRenderPass(window);
		createFramebuffers(window);
		createVertexBuffer(window);
		createShaderModule("Shaders/Simple.vert.spv",
			&context.vertexShaderModule, window.device);
		createShaderModule("Shaders/Simple.frag.spv",
			&context.fragmentShaderModule, window.device);
		createGraphicsPipeline(window);

		while (!glfwWindowShouldClose(window.handle)) {
			render(window);
			glfwWaitEvents();
		}

		quit(window);
	} catch (const exception& e) {
		cerr << e.what() << endl;
		return 1;
	}

	return 0;
}
