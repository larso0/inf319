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
	window(window),
	vertexShaderModule(VK_NULL_HANDLE),
	fragmentShaderModule(VK_NULL_HANDLE)
{
	createShaderModule("Shaders/Simple.vert.spv", &vertexShaderModule,
		window.device);
	createShaderModule("Shaders/Simple.frag.spv", &fragmentShaderModule,
		window.device);
}

VulkanRenderer::~VulkanRenderer() {
	vkDestroyShaderModule(window.device, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(window.device, vertexShaderModule, nullptr);
}

void VulkanRenderer::render(const Engine::Camera& camera,
	const std::vector<Engine::Entity>& entities) {

}
