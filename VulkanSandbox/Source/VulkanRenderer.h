#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include "VulkanWindow.h"
#include "VulkanPerMesh.h"
#include "VulkanShaderProgram.h"
#include "VulkanTexture.h"
#include "VulkanPipeline.h"
#include <Engine/Renderer.h>
#include <unordered_map>
#include <memory>

class VulkanRenderer : public Engine::Renderer {
public:
	VulkanRenderer(VulkanWindow& window);
	~VulkanRenderer();

	void render() override;

	void setTextureAtlas(const Engine::TextureAtlas* atlas) override;

private:
	VulkanWindow& window;

	VulkanShaderProgram program, texturedProgram;
	VulkanPipeline* simplePipeline, * texturedPipeline;
	VulkanBuffer* entityDataBuffer;
	VulkanBuffer* lightDataBuffer;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSet descriptorSet;

	struct EntityData {
		glm::mat4 mvp;
		glm::mat4 normal;
		glm::vec4 color;
		Engine::TextureRect textureRegion;
		glm::vec2 textureScale;
	};

	struct LightData {
		glm::vec3 direction;
	};

	VkDeviceSize entityDataStride, lightDataStride;
	VkSemaphore renderingCompleteSemaphore;
	
	VkSampler textureSampler;
	VulkanTexture* texture;
	VkDescriptorImageInfo imageInfo;
	VkWriteDescriptorSet imageWriteDescriptor;

	std::unordered_map<const Engine::Mesh*, std::shared_ptr<VulkanPerMesh>>
	meshCache;

	void createDescriptorPool();
	void createDescriptorSetLayout();
	void createPipelineLayout();
	void allocateDescriptorSet();
	void createSampler();
	void setupDescriptors();
	void createPipelines();
};

#endif
