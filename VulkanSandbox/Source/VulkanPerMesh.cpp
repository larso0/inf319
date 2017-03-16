#include "VulkanPerMesh.h"
#include <Engine/IndexedMesh.h>
#include <stdexcept>

using namespace std;
using namespace Engine;

VulkanPerMesh::VulkanPerMesh() :
	device(VK_NULL_HANDLE),
	topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST),
	pipeline(VK_NULL_HANDLE),
	indexed(false),
	elementCount(0)
{
}

VulkanPerMesh::~VulkanPerMesh() {
	vkDestroyPipeline(device, pipeline, nullptr);
	for (VulkanBuffer& b : buffers) {
		vkFreeMemory(device, b.memory, nullptr);
		vkDestroyBuffer(device, b.buffer, nullptr);
	}
}

void VulkanPerMesh::init(const VulkanShaderProgram& shaderProgram,
	const Engine::Mesh* mesh,
	const VkPhysicalDeviceMemoryProperties& memoryProperties,
	VkViewport* viewport, VkRect2D* scissor, VkRenderPass renderPass,
	VkPipelineLayout pipelineLayout) {
	device = shaderProgram.getDevice();
	createBuffers(mesh, memoryProperties);

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

	createPipeline(shaderProgram.getShaderStageCreateInfos(), viewport, scissor,
		renderPass, pipelineLayout);
}

void VulkanPerMesh::createBuffers(const Mesh* mesh,
	const VkPhysicalDeviceMemoryProperties& memoryProperties) {
	buffers.push_back(
		createBuffer(device, memoryProperties, mesh->getVertexDataSize(),
			(void*) mesh->getVertexData(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));

	const IndexedMesh* indexedMesh = dynamic_cast<const IndexedMesh*>(mesh);
	if (indexedMesh) {
		indexed = true;
		elementCount = (uint32_t)indexedMesh->getElementCount();
		buffers.push_back(
			createBuffer(device, memoryProperties,
				indexedMesh->getIndexDataSize(),
				(void*) indexedMesh->getIndexData(),
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
	} else {
		elementCount = (uint32_t)mesh->getElementCount();
	}
}

void VulkanPerMesh::createPipeline(
	const vector<VkPipelineShaderStageCreateInfo>& stages, VkViewport* viewport,
	VkRect2D* scissor, VkRenderPass renderPass,
	VkPipelineLayout pipelineLayout) {
	VkVertexInputBindingDescription vertexBindingDescription = {};
	vertexBindingDescription.binding = 0;
	vertexBindingDescription.stride = Vertex::Stride;
	vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription vertexAttributeDescritpions[2];
	vertexAttributeDescritpions[0] = {};
	vertexAttributeDescritpions[0].location = 0;
	vertexAttributeDescritpions[0].binding = 0;
	vertexAttributeDescritpions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexAttributeDescritpions[0].offset = Vertex::PositionOffset;

	vertexAttributeDescritpions[1] = {};
	vertexAttributeDescritpions[1].location = 1;
	vertexAttributeDescritpions[1].binding = 0;
	vertexAttributeDescritpions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexAttributeDescritpions[1].offset = Vertex::NormalOffset;

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
	vertexInputStateCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputStateCreateInfo.pVertexBindingDescriptions =
		&vertexBindingDescription;
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 2;
	vertexInputStateCreateInfo.pVertexAttributeDescriptions =
		vertexAttributeDescritpions;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
	inputAssemblyStateCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCreateInfo.topology = topology;
	inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = scissor;

	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType =
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
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
	pipelineCreateInfo.stageCount = stages.size();
	pipelineCreateInfo.pStages = stages.data();
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
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = nullptr;
	pipelineCreateInfo.basePipelineIndex = 0;

	VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
		&pipelineCreateInfo, nullptr, &pipeline);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create pipeline.");
	}
}

void VulkanPerMesh::record(VkCommandBuffer cmdBuffer) {
	VkDeviceSize offsets = {};
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &buffers[0].buffer, &offsets);

	if (indexed) {
		vkCmdBindIndexBuffer(cmdBuffer, buffers[1].buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmdBuffer, elementCount, 1, 0, 0, 0);
	} else {
		vkCmdDraw(cmdBuffer, elementCount, 1, 0, 0);
	}
}
