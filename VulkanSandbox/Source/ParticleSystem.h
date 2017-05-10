#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <Engine/Node.h>
#include <Engine/Camera.h>
#include <vector>
#include "VulkanDevice.h"
#include "VulkanShaderProgram.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"

class ParticleSystem {
public:
    ParticleSystem(const VulkanDevice& device, VkRenderPass renderPass,
        Engine::Node* emitter, unsigned maxParticles);
    ~ParticleSystem();

    void compute(float deltaTime);
    void recordDraw(const Engine::Camera& camera, VkCommandBuffer cmdBuffer,
        VkViewport vp, VkRect2D scissor);
    void emit(float speed, const glm::vec3& direction);

private:
    Engine::Node* emitter;
    unsigned maxParticles, particleCount, front;

    struct Particle {
        glm::vec3 position;
        float pad0;
        glm::vec3 velocity;
        float pad1;
    };

    struct ComputeInfo {
        float deltaTime;
    };

    uint32_t computeInfoStride;

    struct Matrices {
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
    };

    std::vector<Particle> emitQueue;

    const VulkanDevice& device;
    VkCommandBuffer computeCommandBuffer;
    VulkanShaderProgram computeProgram, drawProgram;
    VulkanBuffer* particleBuffer;
    VulkanBuffer* uniformBuffer;

    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;

    VkPipelineLayout pipelineLayout;
    VkPipeline computePipeline;
    VulkanPipeline* drawPipeline;

    void createDescriptorPool();
    void createDescriptorSetLayout();
    void allocateDescriptorSet();
    void setupDescriptors();
    void createPipelineLayout();
    void createComputePipeline();
    void createDrawPipeline(VkRenderPass renderPass);
};

#endif
