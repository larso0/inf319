#include "ParticleSystem.h"
#include "VulkanUtil.h"
#include <stdexcept>

using namespace std;
using namespace Engine;

ParticleSystem::ParticleSystem(const VulkanDevice& device,
    VkRenderPass renderPass, Engine::Node* emitter, unsigned maxParticles) :
emitter(emitter),
maxParticles(maxParticles),
particleCount(0),
front(0),
device(device),
computeProgram(device),
drawProgram(device)
{
    VkCommandBufferAllocateInfo commandBufferInfo = {};
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferInfo.commandPool = device.getComputeCommandPool();
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.commandBufferCount = 1;
    VkResult result = vkAllocateCommandBuffers(device.getHandle(),
        &commandBufferInfo, &computeCommandBuffer);
    if (result != VK_SUCCESS) {
        throw runtime_error("Failed to allocate compute command buffer.");
    }

    vector<char> computeShaderCode = readBinaryFile("Shaders/Particle.comp.spv");
    vector<char> vertexShaderCode = readBinaryFile("Shaders/Particle.vert.spv");
    vector<char> geometryShaderCode = readBinaryFile("Shaders/Particle.geom.spv");
    vector<char> fragmentShaderCode = readBinaryFile("Shaders/Particle.frag.spv");

    computeProgram.addShaderStage(computeShaderCode, VK_SHADER_STAGE_COMPUTE_BIT);
    drawProgram.addShaderStage(vertexShaderCode, VK_SHADER_STAGE_VERTEX_BIT);
    drawProgram.addShaderStage(geometryShaderCode, VK_SHADER_STAGE_GEOMETRY_BIT);
    drawProgram.addShaderStage(fragmentShaderCode, VK_SHADER_STAGE_FRAGMENT_BIT);

    particleBuffer = new VulkanBuffer(
        device,
        maxParticles * sizeof(Particle),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    );

    computeInfoStride = 0;
	while (computeInfoStride < sizeof(ComputeInfo)) {
		computeInfoStride += device.getProperties().limits
			.minUniformBufferOffsetAlignment;
	}

    uint32_t matricesStride = 0;
    while (matricesStride < sizeof(Matrices)) {
        matricesStride += device.getProperties().limits
            .minUniformBufferOffsetAlignment;
    }

    uniformBuffer = new VulkanBuffer(
        device,
        computeInfoStride + matricesStride,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    );

    createDescriptorPool();
    createDescriptorSetLayout();
    allocateDescriptorSet();
    setupDescriptors();
    createPipelineLayout();
    createComputePipeline();
    createDrawPipeline(renderPass);

    //Test particles
    Particle* mapped = (Particle*)particleBuffer->mapMemory(0, 3*sizeof(Particle));
    mapped[0].position = glm::vec3(1.f, 0.f, 1.f);
    mapped[1].position = glm::vec3(-1.f, 0.f, -1.f);
    mapped[2].position = glm::vec3(0.f, 1.f, 0.f);
    particleBuffer->unmapMemory();

    particleCount = 3;
    front = 3;
}

ParticleSystem::~ParticleSystem() {
    vkDestroyPipelineLayout(device.getHandle(), pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device.getHandle(), descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(device.getHandle(), descriptorPool, nullptr);
    delete particleBuffer;
    delete uniformBuffer;
    vkDestroyPipeline(device.getHandle(), computePipeline, nullptr);
    delete drawPipeline;
}

void ParticleSystem::compute(float deltaTime) {

}

void ParticleSystem::recordDraw(const Engine::Camera& camera,
    VkCommandBuffer cmdBuffer, VkViewport vp, VkRect2D scissor) {
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        drawPipeline->getHandle());
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

    vkCmdSetViewport(cmdBuffer, 0, 1, &vp);
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

    VkDeviceSize offset = 0;
    VkBuffer bufferHandle = particleBuffer->getHandle();
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &bufferHandle, &offset);
    vkCmdDraw(cmdBuffer, particleCount, 1, 0, 0);
}

void ParticleSystem::emit(float speed, const glm::vec3& direction) {
    Particle p;
    p.position = emitter->getPosition();
    p.velocity = quatTransform(emitter->getOrientation(), direction) * speed;
    emitQueue.push_back(p);
}

void ParticleSystem::createDescriptorPool() {
    VkDescriptorPoolSize poolSizes[2];
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = 2;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 1;

    VkResult result = vkCreateDescriptorPool(device.getHandle(), &poolInfo, nullptr,
        &descriptorPool);
    if (result != VK_SUCCESS) {
        throw runtime_error("Failed to create descriptor pool.");
    }
}

void ParticleSystem::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding layoutBindings[3];
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    layoutBindings[0].pImmutableSamplers = nullptr;

    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    layoutBindings[1].pImmutableSamplers = nullptr;

    layoutBindings[2].binding = 2;
    layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[2].descriptorCount = 1;
    layoutBindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT |
        VK_SHADER_STAGE_GEOMETRY_BIT;
    layoutBindings[2].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 3;
    layoutInfo.pBindings = layoutBindings;

    VkResult result = vkCreateDescriptorSetLayout(device.getHandle(), &layoutInfo,
        nullptr, &descriptorSetLayout);
    if (result != VK_SUCCESS) {
        throw runtime_error("Failed to create descriptor set layout.");
    }
}

void ParticleSystem::allocateDescriptorSet() {
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    VkResult result = vkAllocateDescriptorSets(device.getHandle(), &allocInfo,
        &descriptorSet);
    if (result != VK_SUCCESS) {
        throw runtime_error("Failed to allocate description set.");
    }
}

void ParticleSystem::setupDescriptors() {
    VkDescriptorBufferInfo particleBufferInfo = {};
    particleBufferInfo.buffer = particleBuffer->getHandle();
    particleBufferInfo.offset = 0;
    particleBufferInfo.range = maxParticles * sizeof(Particle);

    VkDescriptorBufferInfo computeUniformInfo = {};
    computeUniformInfo.buffer = uniformBuffer->getHandle();
    computeUniformInfo.offset = 0;
    computeUniformInfo.range = sizeof(ComputeInfo);

    VkDescriptorBufferInfo matricesUniformInfo = {};
    matricesUniformInfo.buffer = uniformBuffer->getHandle();
    matricesUniformInfo.offset = computeInfoStride;
    matricesUniformInfo.range = sizeof(Matrices);

    VkWriteDescriptorSet descriptorWrites[3];
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].pNext = nullptr;
    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &particleBufferInfo;
    descriptorWrites[0].pImageInfo = nullptr;
    descriptorWrites[0].pTexelBufferView = nullptr;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].pNext = nullptr;
    descriptorWrites[1].dstSet = descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &computeUniformInfo;
    descriptorWrites[1].pImageInfo = nullptr;
    descriptorWrites[1].pTexelBufferView = nullptr;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].pNext = nullptr;
    descriptorWrites[2].dstSet = descriptorSet;
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = &matricesUniformInfo;
    descriptorWrites[2].pImageInfo = nullptr;
    descriptorWrites[2].pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(device.getHandle(), 3, descriptorWrites, 0, nullptr);
}

void ParticleSystem::createPipelineLayout() {
    VkPipelineLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.setLayoutCount = 1;
    layoutCreateInfo.pSetLayouts = &descriptorSetLayout;
    layoutCreateInfo.pushConstantRangeCount = 0;
    layoutCreateInfo.pPushConstantRanges = nullptr;

    VkResult result = vkCreatePipelineLayout(device.getHandle(), &layoutCreateInfo,
        nullptr, &pipelineLayout);
    if (result != VK_SUCCESS) {
        throw runtime_error("Failed to create pipeline layout.");
    }
}

void ParticleSystem::createComputePipeline() {
    VkComputePipelineCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.stage = computeProgram.getShaderStageCreateInfos()[0];
    createInfo.layout = pipelineLayout;

    VkResult result = vkCreateComputePipelines(device.getHandle(),
        VK_NULL_HANDLE, 1, &createInfo, nullptr, &computePipeline);
    if (result != VK_SUCCESS) {
        throw runtime_error("Failed to create compute pipeline.");
    }
}

void ParticleSystem::createDrawPipeline(VkRenderPass renderPass) {
    VkVertexInputAttributeDescription attrib;
    attrib.location = 0;
    attrib.binding = 0;
    attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
    attrib.offset = 0;
    drawPipeline = new VulkanPipeline(
        drawProgram,
        renderPass,
        pipelineLayout,
        VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
        1, &attrib,
        sizeof(Particle)
    );
}
