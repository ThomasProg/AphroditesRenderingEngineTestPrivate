#pragma once

#include "Test/vk_engine.h"

struct DefaultMaterial : public EngineGraphicsMaterial
{
	static const char* getRawName()
	{
		return "defaultmesh";
	}

	virtual void makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules);
};

struct TestMaterial : public EngineGraphicsMaterial
{
	static const char* getRawName()
	{
		return "testmat";
	}


	virtual void makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules);
};

struct TexturedMaterial : public EngineGraphicsMaterial
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
	virtual void createUniforms(class MemoryManager& memoryManager, VkDescriptorPool _descriptorPool) override;
	virtual void initUniforms(std::unordered_map<std::string, GPUTexture>& _loadedTextures, VkDevice device, VkDescriptorPool _descriptorPool) override;
	virtual void destroyUniforms(class MemoryManager& memoryManager) override;
    virtual void bindUniforms(VkCommandBuffer cmd, VkDescriptorSet& globalDescriptor, VkDescriptorSet& objectDescriptor, int frameIndex, VkPhysicalDeviceProperties _gpuProperties) override;
};

class EdgeDetectMaterial : public ComputeMaterial
{

	void makePipelineLayout(MemoryManager& memoryManager);
	void makePipeline(MemoryManager& memoryManager, const std::unordered_map<std::string, VkShaderModule>& shaderModules);
	
	void createUniforms(MemoryManager& memoryManager, VkDescriptorPool descriptorPool);

	void initUniforms(MemoryManager& memoryManager);
	void destroyUniforms(MemoryManager& memoryManager);

	// VkSampler blockySampler;

public:

	std::unordered_map<std::string, VkShaderModule>* shaderModules = nullptr;
	VkDescriptorPool descriptorPool;

	VkDescriptorImageInfo inImageDescriptor;
	VkDescriptorImageInfo outImageDescriptor;

	virtual void init(MemoryManager& memoryManager) override;
	virtual void destroy() override;
};