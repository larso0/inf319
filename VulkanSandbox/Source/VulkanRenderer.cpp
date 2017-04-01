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
	presentCompleteSemaphore(VK_NULL_HANDLE),
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

	createDescriptorPool();
	createDescriptorSetLayout();
	createPipelineLayout();
	allocateDescriptorSet();
	setupDescriptors();

	VkSemaphoreCreateInfo semaphoreCreateInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 0, 0 };
	vkCreateSemaphore(window.device->getHandle(), &semaphoreCreateInfo, nullptr,
		&presentCompleteSemaphore);
	vkCreateSemaphore(window.device->getHandle(), &semaphoreCreateInfo, nullptr,
		&renderingCompleteSemaphore);
}

VulkanRenderer::~VulkanRenderer() {
	vkDestroyPipelineLayout(window.device->getHandle(), pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(window.device->getHandle(), descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(window.device->getHandle(), descriptorPool, nullptr);
	delete entityDataBuffer;
	delete lightDataBuffer;
	vkDestroySemaphore(window.device->getHandle(), presentCompleteSemaphore, nullptr);
	vkDestroySemaphore(window.device->getHandle(), renderingCompleteSemaphore, nullptr);
}

void VulkanRenderer::render() {
#ifndef NDEBUG
	if (camera == nullptr) {
		throw runtime_error("No camera set.");
	}
#endif

	uint32_t nextImageIdx;
	vkAcquireNextImageKHR(window.device->getHandle(), window.swapchain, UINT64_MAX,
		presentCompleteSemaphore, VK_NULL_HANDLE, &nextImageIdx);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(window.presentCommandBuffer, &beginInfo);

	// change image layout from VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	// to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	VkImageMemoryBarrier layoutTransitionBarrier = {};
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
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = window.renderPass;
	renderPassBeginInfo.framebuffer = window.framebuffers[nextImageIdx];
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
		if (e.getMaterial()) {
			data.color = e.getMaterial()->getColor();
		} else {
			data.color = glm::vec4(0.8f, 0.f, 0.8f, 1.f);
		}
	}
	entityDataBuffer->unmapMemory();

	for (int i = 0; i < entities.size(); i++) {
		const Entity& e = *entities[i];

		const Mesh* mesh = e.getMesh();
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

	VkImageMemoryBarrier prePresentBarrier = {};
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
	vkCreateFence(window.device->getHandle(), &fenceCreateInfo, nullptr, &renderFence);

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
	vkQueueSubmit(window.device->getPresentQueue(), 1, &submitInfo, renderFence);

	vkWaitForFences(window.device->getHandle(), 1, &renderFence, VK_TRUE, UINT64_MAX);
	vkDestroyFence(window.device->getHandle(), renderFence, nullptr);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderingCompleteSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &window.swapchain;
	presentInfo.pImageIndices = &nextImageIdx;
	presentInfo.pResults = nullptr;
	vkQueuePresentKHR(window.device->getPresentQueue(), &presentInfo);
}

void VulkanRenderer::createDescriptorPool() {
	VkDescriptorPoolSize poolSizes[2];
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	poolSizes[0].descriptorCount = 1;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 2;
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = 1;

	VkResult result = vkCreateDescriptorPool(window.device->getHandle(), &poolInfo, nullptr,
		&descriptorPool);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create descriptor pool.");
	}
}

void VulkanRenderer::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding layoutBindings[2];
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
}

void VulkanRenderer::createPipelineLayout() {
	VkPipelineLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutCreateInfo.setLayoutCount = 1;
	layoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	layoutCreateInfo.pushConstantRangeCount = 0;
	layoutCreateInfo.pPushConstantRanges = nullptr;

	VkResult result = vkCreatePipelineLayout(window.device->getHandle(), &layoutCreateInfo,
		nullptr, &pipelineLayout);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create pipeline layout.");
	}
}

void VulkanRenderer::allocateDescriptorSet() {
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	VkResult result = vkAllocateDescriptorSets(window.device->getHandle(), &allocInfo,
		&descriptorSet);
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

	VkWriteDescriptorSet descriptorWrites[2];
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].pNext = nullptr;
	descriptorWrites[0].dstSet = descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &entityBufferInfo;
	descriptorWrites[0].pImageInfo = nullptr;
	descriptorWrites[0].pTexelBufferView = nullptr;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].pNext = nullptr;
	descriptorWrites[1].dstSet = descriptorSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pBufferInfo = &lightBufferInfo;
	descriptorWrites[1].pImageInfo = nullptr;
	descriptorWrites[1].pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(window.device->getHandle(), 2, descriptorWrites, 0, nullptr);
}
