#include "VulkanPerMesh.h"
#include "VulkanRenderer.h"
#include <Engine/IndexedMesh.h>
#include <stdexcept>

using namespace std;
using namespace Engine;

VulkanPerMesh::VulkanPerMesh(VulkanRenderer& renderer, const Mesh* mesh) :
	renderer(renderer),
	topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST),
	pipelineLayout(VK_NULL_HANDLE),
	pipeline(VK_NULL_HANDLE),
	indexed(false),
	indexCount(0)
{
	createBuffers(mesh);
	switch (mesh->getTopology()) {
	case Mesh::Topology::Points:
		topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		break;
	case Mesh::Topology::Lines:
		topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		break;
	case Mesh::Topology::LineStrip:
		topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		break;
	case Mesh::Topology::Triangles:
		topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		break;
	case Mesh::Topology::TriangleStrip:
		topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		break;
	case Mesh::Topology::TriangleFan:
		topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		break;
	}
	createPipeline();
}

VulkanPerMesh::~VulkanPerMesh() {
	vkDestroyPipelineLayout(renderer.window.device, pipelineLayout, nullptr);
	vkDestroyPipeline(renderer.window.device, pipeline, nullptr);
	for (PerBuffer& b : buffers) {
		vkFreeMemory(renderer.window.device, b.memory, nullptr);
		vkDestroyBuffer(renderer.window.device, b.buffer, nullptr);
	}
}

void VulkanPerMesh::createBuffers(const Mesh* mesh) {
	PerBuffer buffer;

	VkBufferCreateInfo vertexBufferInfo = {};
	vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferInfo.size = mesh->getVertexDataSize();
	vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(renderer.window.device, &vertexBufferInfo,
		nullptr, &buffer.buffer);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create vertex buffer.");
	}

	VkMemoryRequirements vertexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(renderer.window.device, buffer.buffer,
		&vertexBufferMemoryRequirements);

	VkMemoryAllocateInfo bufferAllocateInfo = {};
	bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	bufferAllocateInfo.allocationSize = vertexBufferMemoryRequirements.size;

	uint32_t vertexMemoryTypeBits = vertexBufferMemoryRequirements
		.memoryTypeBits;
	VkMemoryPropertyFlags vertexDesiredMemoryFlags =
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	for (uint32_t i = 0; i < 32; ++i) {
		VkMemoryType memoryType =
			renderer.window.memoryProperties.memoryTypes[i];
		if (vertexMemoryTypeBits & 1) {
			if ((memoryType.propertyFlags & vertexDesiredMemoryFlags)
				== vertexDesiredMemoryFlags) {
				bufferAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vertexMemoryTypeBits = vertexMemoryTypeBits >> 1;
	}

	result = vkAllocateMemory(renderer.window.device, &bufferAllocateInfo,
		nullptr, &buffer.memory);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to allocate vertex buffer memory.");
	}

	void* mapped;
	result = vkMapMemory(renderer.window.device, buffer.memory, 0,
		VK_WHOLE_SIZE, 0, &mapped);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to map vertex buffer memory.");
	}

	memcpy(mapped, mesh->getVertexData(), mesh->getVertexDataSize());
	vkUnmapMemory(renderer.window.device, buffer.memory);

	result = vkBindBufferMemory(renderer.window.device, buffer.buffer,
		buffer.memory, 0);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to bind vertex buffer memory.");
	}

	buffers.push_back(buffer);

	const IndexedMesh* indexedMesh = dynamic_cast<const IndexedMesh*>(mesh);
	if (indexedMesh) {
		indexed = true;
		indexCount = (uint32_t)indexedMesh->getElementCount();
		PerBuffer buffer;
		VkBufferCreateInfo indexBufferInfo = {};
		indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		indexBufferInfo.size = indexedMesh->getIndexDataSize();
		indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(renderer.window.device,
			&indexBufferInfo, nullptr, &buffer.buffer);
		if (result != VK_SUCCESS) {
			throw runtime_error("Failed to create index buffer.");
		}

		VkMemoryRequirements requirements = {};
		vkGetBufferMemoryRequirements(renderer.window.device, buffer.buffer,
			&requirements);
		bufferAllocateInfo.allocationSize = requirements.size;
		result = vkAllocateMemory(renderer.window.device, &bufferAllocateInfo,
			nullptr, &buffer.memory);
		if (result != VK_SUCCESS) {
			throw runtime_error("Failed to allocate index buffer memory.");
		}

		result = vkMapMemory(renderer.window.device, buffer.memory, 0,
			VK_WHOLE_SIZE, 0, &mapped);
		if (result != VK_SUCCESS) {
			throw runtime_error("Failed to map vertex buffer memory.");
		}

		memcpy(mapped, indexedMesh->getIndexData(),
			indexedMesh->getIndexDataSize());
		vkUnmapMemory(renderer.window.device, buffer.memory);

		result = vkBindBufferMemory(renderer.window.device, buffer.buffer,
			buffer.memory, 0);
		if (result != VK_SUCCESS) {
			throw runtime_error("Failed to bind index buffer memory.");
		}

		buffers.push_back(buffer);
	}
}

void VulkanPerMesh::createPipeline() {
	VkPipelineLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutCreateInfo.setLayoutCount = 0;
	layoutCreateInfo.pSetLayouts = nullptr;
	layoutCreateInfo.pushConstantRangeCount = 0;
	layoutCreateInfo.pPushConstantRanges = nullptr;

	VkResult result = vkCreatePipelineLayout(renderer.window.device,
		&layoutCreateInfo, nullptr, &pipelineLayout);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create pipeline layout.");
	}

	VkPipelineShaderStageCreateInfo shaderStageCreateInfo[2] = {};
	shaderStageCreateInfo[0].sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageCreateInfo[0].module = renderer.vertexShaderModule;
	shaderStageCreateInfo[0].pName = "main";
	shaderStageCreateInfo[0].pSpecializationInfo = nullptr;

	shaderStageCreateInfo[1].sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageCreateInfo[1].module = renderer.fragmentShaderModule;
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
	inputAssemblyStateCreateInfo.topology = topology;
	inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = renderer.window.width;
	viewport.height = renderer.window.height;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;

	VkRect2D scissors = {};
	scissors.offset = { 0, 0 };
	scissors.extent = { renderer.window.width, renderer.window.height };

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
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderer.window.renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = nullptr;
	pipelineCreateInfo.basePipelineIndex = 0;

	result = vkCreateGraphicsPipelines(renderer.window.device, VK_NULL_HANDLE,
		1, &pipelineCreateInfo, nullptr, &pipeline);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create pipeline.");
	}
}

void VulkanPerMesh::record(VkCommandBuffer cmdBuffer) {
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	VkViewport viewport = { 0, 0, (float) renderer.window.width,
		(float) renderer.window.height, 0, 1 };
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

	VkRect2D scissor = { 0, 0, renderer.window.width, renderer.window.height };
	vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

	VkDeviceSize offsets = {};
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &buffers[0].buffer, &offsets);

	if (indexed) {
		vkCmdBindIndexBuffer(cmdBuffer, buffers[1].buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmdBuffer, indexCount, 1, 0, 0, 0);
	} else {
		vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
	}
}
