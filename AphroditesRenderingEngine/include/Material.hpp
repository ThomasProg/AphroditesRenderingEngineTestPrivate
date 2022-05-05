#pragma once


#include "Vk/Instance.hpp"
#include "PhysicalDevice.hpp"
#include <vk_types.h>

#include <unordered_map>
#include "Test/vk_texture.hpp"

class MemoryManager;

class Material
{
protected:
	MemoryManager* memoryManager = nullptr;

public: // must be protected
	// Pipeline and its layout
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

public:
	virtual void init(MemoryManager& memoryManager) 
	{
		this->memoryManager = &memoryManager;
	}
	virtual void bind(VkCommandBuffer cmd) = 0;
	virtual void destroy();
};

class ComputeMaterial : public Material
{
protected:
	using Super = Material;
	VkDescriptorSetLayout shaderLayout = VK_NULL_HANDLE;
	VkDescriptorSet matDescriptors = VK_NULL_HANDLE;

public:
    virtual void bind(VkCommandBuffer cmd) override;
};

class GraphicsMaterial : public Material
{
protected:
	using Super = Material;

};

class PipelineBuilder 
{
public:

	std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
	VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
	VkViewport _viewport;
	VkRect2D _scissor;
	VkPipelineRasterizationStateCreateInfo _rasterizer;
	VkPipelineColorBlendAttachmentState _colorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo _multisampling;
	VkPipelineLayout _pipelineLayout;
	VkPipelineDepthStencilStateCreateInfo _depthStencil;
	VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
};

class EngineGraphicsMaterial : public GraphicsMaterial
{
public:
	static PipelineBuilder getPipelineBuilder(float width, float height);

public:
	void destroyPipeline(MemoryManager& memoryManager);

	// Uniforms
	virtual void updateUniforms(VmaAllocator _allocator, int frameID) {}
	// virtual void createUniforms(class VulkanEngine& context, VkDevice device, VkDescriptorPool _descriptorPool) {}
	virtual void createUniforms(class MemoryManager& memoryManager, VkDescriptorPool _descriptorPool) {}
	virtual void initUniforms(std::unordered_map<std::string, GPUTexture>& _loadedTextures, VkDevice device, VkDescriptorPool _descriptorPool) {}
	virtual void destroyUniforms(class MemoryManager& memoryManager) {}
	virtual void bindUniforms(VkCommandBuffer cmd, VkDescriptorSet& globalDescriptor, VkDescriptorSet& objectDescriptor, int frameIndex, VkPhysicalDeviceProperties _gpuProperties);

	// Pipeline
	void makePipelineLayout(VkDevice _device, VkDescriptorSetLayout* layouts, int nbLayouts);
	virtual void makePipelineLayout(VkDevice _device, VkDescriptorSetLayout _globalSetLayout, VkDescriptorSetLayout _objectSetLayout);
	virtual void makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules) {}
	virtual void makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules, const std::string& vertexShader, const std::string& fragmentShader);

public:
	//virtual void init(MemoryManager& memoryManager) override{}
	virtual void bind(VkCommandBuffer cmd) override{}
	virtual void destroy() override{}
};