#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include "VulkanWindow.h"
#include "VulkanPerMesh.h"
#include "VulkanShaderProgram.h"
#include "VulkanTexture.h"
#include <Engine/Renderer.h>
#include <unordered_map>
#include <memory>

class VulkanRenderer : public Engine::Renderer {
public:
	VulkanRenderer(VulkanWindow& window);
	~VulkanRenderer();

	void render() override;

private:
	VulkanWindow& window;

	VulkanShaderProgram program;
	VulkanBuffer* entityDataBuffer;
	VulkanBuffer* lightDataBuffer;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSetLayout texturedDescriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSet descriptorSets[2];

	struct EntityData {
		glm::mat4 mvp;
		glm::mat4 normal;
		glm::vec4 color;
	};

	struct LightData {
		glm::vec3 direction;
	};

	VkDeviceSize entityDataStride, lightDataStride;
	VkSemaphore renderingCompleteSemaphore;
	
	VkSampler textureSampler;
	
	VulkanTexture* texture;

	std::unordered_map<const Engine::Mesh*, std::shared_ptr<VulkanPerMesh>>
	meshCache;

	void createDescriptorPool();
	void createDescriptorSetLayouts();
	void createPipelineLayout();
	void allocateDescriptorSets();
	void createSampler();
	void setupDescriptors();
};

#endif
