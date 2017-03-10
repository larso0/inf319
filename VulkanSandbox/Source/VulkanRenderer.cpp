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

static void createShaderModule(const string& filename, VkShaderModule* dst,
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

VulkanRenderer::VulkanRenderer(VulkanWindow& window) :
	window(window)
{
	vector<char> vertexShaderCode = readFile("Shaders/Simple.vert.spv");
	vector<char> fragmentShaderCode = readFile("Shaders/Simple.frag.spv");
	program.setDevice(window.device);
	program.addShaderStage(vertexShaderCode, VK_SHADER_STAGE_VERTEX_BIT);
	program.addShaderStage(fragmentShaderCode, VK_SHADER_STAGE_FRAGMENT_BIT);
}

VulkanRenderer::~VulkanRenderer() {
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

	for (const Entity& e : entities) {
		const Mesh* mesh = e.getMesh();
		auto result = meshCache.find(mesh);
		if (result == meshCache.end()) {
			shared_ptr<VulkanPerMesh> perMesh = make_shared<VulkanPerMesh>();
			perMesh->init(program, mesh, window.memoryProperties,
				&window.viewport, &window.scissor, window.renderPass);
			meshCache[mesh] = perMesh;
		}

		meshCache[mesh]->record(window.presentCommandBuffer, &window.viewport,
			&window.scissor);
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
