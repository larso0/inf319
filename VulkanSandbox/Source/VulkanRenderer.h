#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include "VulkanWindow.h"
#include "VulkanPerMesh.h"
#include "VulkanShaderProgram.h"
#include <Engine/Renderer.h>
#include <unordered_map>
#include <memory>

class VulkanRenderer : public Engine::Renderer {
public:
	VulkanRenderer(VulkanWindow& window);
	~VulkanRenderer();

	void render(const Engine::Camera& camera,
		const std::vector<Engine::Entity>& entities) override;

private:
	VulkanWindow& window;

	VulkanShaderProgram program;

	std::unordered_map<const Engine::Mesh*, std::shared_ptr<VulkanPerMesh>>
	meshCache;
};

#endif
