#pragma once

#include "Test/vk_engine.h"

struct DefaultMaterial : public GraphicsMaterial
{
	static const char* getRawName()
	{
		return "defaultmesh";
	}

	virtual void makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules);
};

struct TestMaterial : public GraphicsMaterial
{
	static const char* getRawName()
	{
		return "testmat";
	}


	virtual void makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules);
};

struct TexturedMaterial : public GraphicsMaterial
{
	// uniform buffer for each frame
	std::vector<AllocatedBuffer> uniformBuffers;

	// Contains textures' and uniforms' descriptor sets
	VkDescriptorSetLayout shaderLayout = VK_NULL_HANDLE;
	VkDescriptorSet matDescriptors{VK_NULL_HANDLE};

	VkSampler blockySampler;

	static const char* getRawName()
	{
		return "texturedmesh";
	}

	virtual void makePipelineLayout(VkDevice _device, VkDescriptorSetLayout _globalSetLayout, VkDescriptorSetLayout _objectSetLayout) override;
	virtual void makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules) override;
	
    virtual void updateUniforms(VmaAllocator _allocator, int frameID) override;
    virtual void createUniforms(VulkanEngine& context, VkDevice device, VkDescriptorPool _descriptorPool) override;
	virtual void initUniforms(class VulkanEngine& context, VkDevice device, VkDescriptorPool _descriptorPool) override;
	virtual void destroyUniforms(class MemoryManager& memoryManager) override;
    virtual void bindUniforms(VkCommandBuffer cmd, VkDescriptorSet& globalDescriptor, VkDescriptorSet& objectDescriptor, int frameIndex, VkPhysicalDeviceProperties _gpuProperties) override;
};

struct EdgeDetectMaterial : public ComputeMaterial
{
	VkDescriptorImageInfo inImageDescriptor;
	VkDescriptorImageInfo outImageDescriptor;

	virtual void makePipelineLayout(MemoryManager& memoryManager) override;
	virtual void makePipeline(MemoryManager& memoryManager, const std::unordered_map<std::string, VkShaderModule>& shaderModules) override;
	
	virtual void createUniforms(MemoryManager& memoryManager, VkDescriptorPool descriptorPool) override;

	void initUniforms(MemoryManager& memoryManager);
	virtual void destroyUniforms(MemoryManager& memoryManager) override;
    virtual void bind(MemoryManager& memoryManager, VkCommandBuffer cmd) override;
};