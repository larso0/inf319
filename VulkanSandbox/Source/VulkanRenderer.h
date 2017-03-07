#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include "VulkanWindow.h"
#include <Engine/Renderer.h>

class VulkanRenderer : public Engine::Renderer {
public:
	VulkanRenderer(VulkanWindow& window);
	~VulkanRenderer();

	void render(const Engine::Camera& camera,
		const std::vector<Engine::Entity>& entities) override;

//private:
	VulkanWindow& window;

	VkShaderModule vertexShaderModule;
	VkShaderModule fragmentShaderModule;
};

#endif
