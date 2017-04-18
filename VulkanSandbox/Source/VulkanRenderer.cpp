#include "VulkanRenderer.h"
#include <string>
#include <vector>
#include <fstream>

using namespace std;
using namespace Engine;

static vector<char> readFile(const string& filename) {
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

VulkanRenderer::VulkanRenderer(VulkanWindow& window) :
	Renderer(),
	window(window),
	program(VulkanShaderProgram(*window.device)),
	descriptorPool(VK_NULL_HANDLE),
	renderingCompleteSemaphore(VK_NULL_HANDLE)
{
	vector<char> vertexShaderCode = readFile("Shaders/Simple.vert.spv");
	vector<char> fragmentShaderCode = readFile("Shaders/Simple.frag.spv");
	program.addShaderStage(vertexShaderCode, VK_SHADER_STAGE_VERTEX_BIT);
	program.addShaderStage(fragmentShaderCode, VK_SHADER_STAGE_FRAGMENT_BIT);

	entityDataStride = 0;
	while (entityDataStride < sizeof(EntityData)) {
		entityDataStride += window.device->getProperties().limits
			.minUniformBufferOffsetAlignment;
	}

	lightDataStride = 0;
	while (lightDataStride < sizeof(LightData)) {
		lightDataStride += window.device->getProperties().limits
			.minUniformBufferOffsetAlignment;
	}

	VkDeviceSize entityDataBufferSize = entityDataStride * 64;
	entityDataBuffer = new VulkanBuffer(*window.device, entityDataBufferSize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			| VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	lightDataBuffer = new VulkanBuffer(*window.device, lightDataStride,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			| VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	
	Engine::Texture tex("../Assets/texture.jpg");
	texture = new VulkanTexture(*window.device, &tex);

	createDescriptorPool();
	createDescriptorSetLayouts();
	createPipelineLayout();
	allocateDescriptorSets();
	setupDescriptors();

	VkSemaphoreCreateInfo semaphoreCreateInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 0, 0 };
	vkCreateSemaphore(window.device->getHandle(), &semaphoreCreateInfo, nullptr,
		&renderingCompleteSemaphore);
	
	createSampler();
}

VulkanRenderer::~VulkanRenderer() {
	vkDestroyPipelineLayout(window.device->getHandle(), pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(window.device->getHandle(), descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(window.device->getHandle(), descriptorPool, nullptr);
	delete entityDataBuffer;
	delete lightDataBuffer;
	delete texture;
	vkDestroySemaphore(window.device->getHandle(), renderingCompleteSemaphore, nullptr);
}

void VulkanRenderer::render() {
#ifndef NDEBUG
	if (camera == nullptr) {
		throw runtime_error("No camera set.");
	}
#endif
	
	window.swapchain->nextImage();

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(window.presentCommandBuffer, &beginInfo);
	
	window.swapchain->transitionColor(window.presentCommandBuffer);

	VkClearValue clearValue[] = { { 0.5f, 0.5f, 0.5f, 1.f }, { 1.f, 0.f } };
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = window.renderPass;
	renderPassBeginInfo.framebuffer =
		window.framebuffers[window.swapchain->getCurrentImageIndex()];
	renderPassBeginInfo.renderArea = {0, 0, window.getWidth(), window.getHeight()};
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValue;
	vkCmdBeginRenderPass(window.presentCommandBuffer, &renderPassBeginInfo,
		VK_SUBPASS_CONTENTS_INLINE);

	if (lightSources.empty()) {
		throw runtime_error("No light source.");
	}

	void* mapped = lightDataBuffer->mapMemory(0, lightDataStride);
	LightData& lightData = *((LightData*) ((char*) mapped));
	lightData.direction = lightSources[0]->getDirection();
	lightDataBuffer->unmapMemory();

	VkDeviceSize neededSize = entities.size() * entityDataStride;
	mapped = entityDataBuffer->mapMemory(0, neededSize);
	for (int i = 0; i < entities.size(); i++) {
		const Entity& e = *entities[i];
		EntityData& data = *((EntityData*) ((char*) mapped
			+ i * entityDataStride));
		glm::mat4 worldMatrix = e.getNode()->getWorldMatrix()
			* e.getScaleMatrix();
		data.mvp = camera->getProjectionMatrix() * camera->getViewMatrix()
			* worldMatrix;
		data.normal = glm::transpose(glm::inverse(worldMatrix));
		if (e.getGeometry()->getMaterial()) {
			data.color = e.getGeometry()->getMaterial()->getColor();
		} else {
			data.color = glm::vec4(0.8f, 0.f, 0.8f, 1.f);
		}
	}
	entityDataBuffer->unmapMemory();

	for (int i = 0; i < entities.size(); i++) {
		const Entity& e = *entities[i];

		const Mesh* mesh = e.getGeometry()->getMesh();
		auto found = meshCache.find(mesh);
		if (found == meshCache.end()) {
			shared_ptr<VulkanPerMesh> perMesh = make_shared<VulkanPerMesh>();
			perMesh->init(program, mesh, window.device->getMemoryProperties(),
				&window.viewport, &window.scissor, window.renderPass,
				pipelineLayout);
			meshCache[mesh] = perMesh;
		}

		shared_ptr<VulkanPerMesh>& perMesh = meshCache[mesh];

		vkCmdBindPipeline(window.presentCommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS, perMesh->getPipeline());

		uint32_t uniformOffset = i * entityDataStride;

		vkCmdBindDescriptorSets(window.presentCommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
			&descriptorSet, 1, &uniformOffset);

		vkCmdSetViewport(window.presentCommandBuffer, 0, 1, &window.viewport);
		vkCmdSetScissor(window.presentCommandBuffer, 0, 1, &window.scissor);

		perMesh->record(window.presentCommandBuffer);
	}

	vkCmdEndRenderPass(window.presentCommandBuffer);
	
	window.swapchain->transitionPresent(window.presentCommandBuffer);

	vkEndCommandBuffer(window.presentCommandBuffer);

	VkFence renderFence;
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(window.device->getHandle(), &fenceCreateInfo, nullptr, &renderFence);

	VkSemaphore presentSemaphore = window.swapchain->getPresentSemaphore();
	
	VkPipelineStageFlags waitStageMash =
		{ VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &presentSemaphore;
	submitInfo.pWaitDstStageMask = &waitStageMash;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &window.presentCommandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderingCompleteSemaphore;
	vkQueueSubmit(window.device->getPresentQueue(), 1, &submitInfo, renderFence);

	vkWaitForFences(window.device->getHandle(), 1, &renderFence, VK_TRUE, UINT64_MAX);
	vkDestroyFence(window.device->getHandle(), renderFence, nullptr);
	
	window.swapchain->present(renderingCompleteSemaphore);
}

void VulkanRenderer::createDescriptorPool() {
	VkDescriptorPoolSize poolSizes[3];
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	poolSizes[0].descriptorCount = 2;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = 2;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 3;
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = 1;

	VkResult result = vkCreateDescriptorPool(window.device->getHandle(), &poolInfo, nullptr,
		&descriptorPool);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create descriptor pool.");
	}
}

void VulkanRenderer::createDescriptorSetLayouts() {
    VkDescriptorSetLayoutBinding layoutBindings[3];
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBindings[0].pImmutableSamplers = nullptr;

    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layoutBindings[1].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = layoutBindings;

	VkResult result = vkCreateDescriptorSetLayout(window.device->getHandle(), &layoutInfo,
		nullptr, &descriptorSetLayout);
    if (result != VK_SUCCESS) {
    	throw runtime_error("Failed to create descriptor set layout.");
    }
    
    layoutBindings[2].binding = 2;
	layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindings[2].descriptorCount = 1;
	layoutBindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindings[2].pImmutableSamplers = nullptr;
	
	layoutInfo.bindingCount = 3;
	
	result = vkCreateDescriptorSetLayout(window.device->getHandle(), &layoutInfo,
		nullptr, &texturedDescriptorSetLayout);
    if (result != VK_SUCCESS) {
    	throw runtime_error("Failed to create textured descriptor set layout.");
    }
}

void VulkanRenderer::createPipelineLayout() {
	VkDescriptorSetLayout setLayouts[] = {
		descriptorSetLayout,
		texturedDescriptorSetLayout
	};
	
	VkPipelineLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutCreateInfo.setLayoutCount = 2;
	layoutCreateInfo.pSetLayouts = setLayouts;
	layoutCreateInfo.pushConstantRangeCount = 0;
	layoutCreateInfo.pPushConstantRanges = nullptr;

	VkResult result = vkCreatePipelineLayout(window.device->getHandle(), &layoutCreateInfo,
		nullptr, &pipelineLayout);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create pipeline layout.");
	}
}

void VulkanRenderer::allocateDescriptorSets() {
	VkDescriptorSetLayout setLayouts[] = {
		descriptorSetLayout,
		texturedDescriptorSetLayout
	};
	
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 2;
	allocInfo.pSetLayouts = setLayouts;

	VkResult result = vkAllocateDescriptorSets(
		window.device->getHandle(), &allocInfo, descriptorSets);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to allocate description set.");
	}
}

void VulkanRenderer::setupDescriptors() {
	VkDescriptorBufferInfo entityBufferInfo = {};
	entityBufferInfo.buffer = entityDataBuffer->getHandle();
	entityBufferInfo.offset = 0;
	entityBufferInfo.range = entityDataStride;

	VkDescriptorBufferInfo lightBufferInfo = {};
	lightBufferInfo.buffer = lightDataBuffer->getHandle();
	lightBufferInfo.offset = 0;
	lightBufferInfo.range = lightDataStride;
	
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = texture->getImageView();
	imageInfo.sampler = textureSampler;

	VkWriteDescriptorSet descriptorWrites[5];
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].pNext = nullptr;
	descriptorWrites[0].dstSet = descriptorSets[0];
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &entityBufferInfo;
	descriptorWrites[0].pImageInfo = nullptr;
	descriptorWrites[0].pTexelBufferView = nullptr;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].pNext = nullptr;
	descriptorWrites[1].dstSet = descriptorSets[0];
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pBufferInfo = &lightBufferInfo;
	descriptorWrites[1].pImageInfo = nullptr;
	descriptorWrites[1].pTexelBufferView = nullptr;
	
	descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[2].pNext = nullptr;
	descriptorWrites[2].dstSet = descriptorSets[1];
	descriptorWrites[2].dstBinding = 0;
	descriptorWrites[2].dstArrayElement = 0;
	descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	descriptorWrites[2].descriptorCount = 1;
	descriptorWrites[2].pBufferInfo = &entityBufferInfo;
	descriptorWrites[2].pImageInfo = nullptr;
	descriptorWrites[2].pTexelBufferView = nullptr;

	descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[3].pNext = nullptr;
	descriptorWrites[3].dstSet = descriptorSets[1];
	descriptorWrites[3].dstBinding = 1;
	descriptorWrites[3].dstArrayElement = 0;
	descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[3].descriptorCount = 1;
	descriptorWrites[3].pBufferInfo = &lightBufferInfo;
	descriptorWrites[3].pImageInfo = nullptr;
	descriptorWrites[3].pTexelBufferView = nullptr;
	
	descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[4].pNext = nullptr;
	descriptorWrites[4].dstSet = descriptorSets[1];
	descriptorWrites[4].dstBinding = 2;
	descriptorWrites[4].dstArrayElement = 0;
	descriptorWrites[4].descriptorType =
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[4].descriptorCount = 1;
	descriptorWrites[4].pBufferInfo = nullptr;
	descriptorWrites[4].pImageInfo = &imageInfo;
	descriptorWrites[4].pTexelBufferView = nullptr;
	
	vkUpdateDescriptorSets(window.device->getHandle(), 5, descriptorWrites, 0, nullptr);
}

void VulkanRenderer::createSampler() {
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	
	VkResult result = vkCreateSampler(
		window.device->getHandle(), &samplerInfo, nullptr, &textureSampler);
	if (result != VK_SUCCESS) {
		
	}
}
