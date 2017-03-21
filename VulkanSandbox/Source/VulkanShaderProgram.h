#ifndef VULKANSHADERPROGRAM_H
#define VULKANSHADERPROGRAM_H

#include <vulkan/vulkan.h>
#include "VulkanDevice.h"
#include <vector>

class VulkanShaderProgram {
public:
	VulkanShaderProgram(const VulkanDevice& device);
	~VulkanShaderProgram();

	void addShaderStage(const std::vector<char>& spirvCode,
		VkShaderStageFlagBits stage);

	const std::vector<VkPipelineShaderStageCreateInfo>&
	getShaderStageCreateInfos() const {
		return shaderStages;
	}

	const VulkanDevice& getDevice() const {
		return device;
	}

private:
	const VulkanDevice& device;
	std::vector<VkShaderModule> shaderModules;
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
};

#endif
