#pragma once


#include "Vk/Instance.hpp"
#include "PhysicalDevice.hpp"
#include <vk_types.h>

#include <unordered_map>

class MemoryManager;

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


struct Material 
{
protected:
	// const MemoryManager& memoryManager;

public:
	// Pipeline and its layout
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;

	void destroyPipeline(MemoryManager& memoryManager);

	// Material(MemoryManager& memoryManager) : memoryManager(memoryManager) {}
	// virtual void bind(MemoryManager& memoryManager, VkCommandBuffer cmd) = 0;
	// virtual ~Material() = default;
};

struct ComputeMaterial : public Material
{
	VkDescriptorSetLayout shaderLayout = VK_NULL_HANDLE;
	VkDescriptorSet matDescriptors{VK_NULL_HANDLE};

public:
	// ComputeMaterial();
    virtual void bind(MemoryManager& memoryManager, VkCommandBuffer cmd) {}
	// virtual ~ComputeMaterial();

private:
	virtual void makePipelineLayout(MemoryManager& memoryManager) {}
	virtual void makePipeline(MemoryManager& memoryManager, const std::unordered_map<std::string, VkShaderModule>& shaderModules) {}
	virtual void createUniforms(MemoryManager& memoryManager, VkDescriptorPool descriptorPool) {}
	virtual void initUniforms(MemoryManager& memoryManager) {}
	virtual void destroyUniforms(MemoryManager& memoryManager) {}
};

struct GraphicsMaterial : public Material
{

	static PipelineBuilder getPipelineBuilder(class Window& window);

	// Uniforms
	virtual void updateUniforms(VmaAllocator _allocator, int frameID) {}
	virtual void createUniforms(class VulkanEngine& context, VkDevice device, VkDescriptorPool _descriptorPool) {}
	virtual void initUniforms(class VulkanEngine& context, VkDevice device, VkDescriptorPool _descriptorPool) {}
	virtual void destroyUniforms(class MemoryManager& memoryManager) {}
	virtual void bindUniforms(VkCommandBuffer cmd, VkDescriptorSet& globalDescriptor, VkDescriptorSet& objectDescriptor, int frameIndex, VkPhysicalDeviceProperties _gpuProperties);

	// Pipeline
	void makePipelineLayout(VkDevice _device, VkDescriptorSetLayout* layouts, int nbLayouts);
	virtual void makePipelineLayout(VkDevice _device, VkDescriptorSetLayout _globalSetLayout, VkDescriptorSetLayout _objectSetLayout);
	virtual void makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules) {}
	virtual void makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules, const std::string& vertexShader, const std::string& fragmentShader);
};


