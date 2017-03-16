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
	window(window),
	descriptorPool(VK_NULL_HANDLE)
{
	vector<char> vertexShaderCode = readFile("Shaders/Simple.vert.spv");
	vector<char> fragmentShaderCode = readFile("Shaders/Simple.frag.spv");
	program.setDevice(window.device);
	program.addShaderStage(vertexShaderCode, VK_SHADER_STAGE_VERTEX_BIT);
	program.addShaderStage(fragmentShaderCode, VK_SHADER_STAGE_FRAGMENT_BIT);
	uniformBuffer = createBuffer(window.device, window.memoryProperties,
		sizeof(Matrices) * 512, nullptr, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	createDescriptorPool();
	createDescriptorSetLayout();
	createPipelineLayout();
	allocateDescriptorSet();
	setupDescriptors();
}

VulkanRenderer::~VulkanRenderer() {
	vkDestroyPipelineLayout(window.device, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(window.device, descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(window.device, descriptorPool, nullptr);
	vkFreeMemory(window.device, uniformBuffer.memory, nullptr);
	vkDestroyBuffer(window.device, uniformBuffer.buffer, nullptr);
}

void VulkanRenderer::render(const Engine::Camera& camera,
	const std::vector<Engine::Entity>& entities) {
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

	void* mapped;
	VkResult result = vkMapMemory(window.device, uniformBuffer.memory, 0,
		VK_WHOLE_SIZE, 0, &mapped);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to map uniform buffer memory.");
	}

	for (int i = 0; i < entities.size(); i++) {
		const Entity& e = entities[i];

		Matrices& m = *((Matrices*)mapped + i);
		glm::mat4 worldMatrix = e.getNode()->getWorldMatrix()
			* e.getScaleMatrix();
		m.mvp = camera.getProjectionMatrix() * camera.getViewMatrix()
			* worldMatrix;
		m.normal = glm::transpose(glm::inverse(worldMatrix));
	}

	vkUnmapMemory(window.device, uniformBuffer.memory);

	for (int i = 0; i < entities.size(); i++) {
		const Entity& e = entities[i];

		const Mesh* mesh = e.getMesh();
		auto found = meshCache.find(mesh);
		if (found == meshCache.end()) {
			shared_ptr<VulkanPerMesh> perMesh = make_shared<VulkanPerMesh>();
			perMesh->init(program, mesh, window.memoryProperties,
				&window.viewport, &window.scissor, window.renderPass,
				pipelineLayout);
			meshCache[mesh] = perMesh;
		}

		shared_ptr<VulkanPerMesh>& perMesh = meshCache[mesh];

		vkCmdBindPipeline(window.presentCommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS, perMesh->getPipeline());

		uint32_t uniformOffset = i * sizeof(Matrices);

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

void VulkanRenderer::createDescriptorPool() {
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;

	VkResult result = vkCreateDescriptorPool(window.device, &poolInfo, nullptr,
		&descriptorPool);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to create descriptor pool.");
	}
}

void VulkanRenderer::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

	VkResult result = vkCreateDescriptorSetLayout(window.device, &layoutInfo,
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

	VkResult result = vkCreatePipelineLayout(window.device, &layoutCreateInfo,
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

	VkResult result = vkAllocateDescriptorSets(window.device, &allocInfo,
		&descriptorSet);
	if (result != VK_SUCCESS) {
		throw runtime_error("Failed to allocate description set.");
	}
}

void VulkanRenderer::setupDescriptors() {
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = uniformBuffer.buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(Matrices);

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = nullptr;
	descriptorWrite.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(window.device, 1, &descriptorWrite, 0, nullptr);
}
