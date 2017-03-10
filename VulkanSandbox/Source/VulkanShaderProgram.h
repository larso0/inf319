#ifndef VULKANSHADERPROGRAM_H
#define VULKANSHADERPROGRAM_H

#include <vulkan/vulkan.h>
#include <vector>

class VulkanShaderProgram {
public:
	VulkanShaderProgram();
	VulkanShaderProgram(VkDevice device);
	~VulkanShaderProgram();

	void addShaderStage(const std::vector<char>& spirvCode,
		VkShaderStageFlagBits stage);

	const std::vector<VkPipelineShaderStageCreateInfo>&
	getShaderStageCreateInfos() const {
		return shaderStages;
	}

	VkDevice getDevice() const {
		return device;
	}

	void setDevice(VkDevice device) {
		this->device = device;
	}

private:
	VkDevice device;
	std::vector<VkShaderModule> shaderModules;
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
};

#endif
