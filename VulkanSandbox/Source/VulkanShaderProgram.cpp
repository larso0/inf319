#include "VulkanShaderProgram.h"
#include <stdexcept>

VulkanShaderProgram::VulkanShaderProgram() :
	device(VK_NULL_HANDLE)
{}

VulkanShaderProgram::VulkanShaderProgram(VkDevice device) :
	device(device)
{}

VulkanShaderProgram::~VulkanShaderProgram() {
	for (VkShaderModule m : shaderModules) {
		vkDestroyShaderModule(device, m, nullptr);
	}
}

void VulkanShaderProgram::addShaderStage(const std::vector<char>& spirvCode,
	VkShaderStageFlagBits stage) {
	VkShaderModule module;

	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = spirvCode.size();
	info.pCode = (uint32_t *)spirvCode.data();

	VkResult result = vkCreateShaderModule(device, &info, nullptr, &module);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module.");
	}

	shaderModules.push_back(module);

	VkPipelineShaderStageCreateInfo stageInfo = {};
	stageInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageInfo.stage = stage;
	stageInfo.module = module;
	stageInfo.pName = "main";
	stageInfo.pSpecializationInfo = nullptr;

	shaderStages.push_back(stageInfo);
}
