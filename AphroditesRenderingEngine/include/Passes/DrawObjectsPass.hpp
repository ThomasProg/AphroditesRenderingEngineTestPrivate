#pragma once

#include "Passes/Pass.hpp"
#include <vk_types.h>
#include "RenderObject.hpp"
#include "Test/ResourceManager.hpp"

class MemoryManager;

struct GPUSceneData 
{
	glm::vec4 fogColor; // w is for exponent
	glm::vec4 fogDistances; //x for min, y for max, zw unused.
	glm::vec4 ambientColor;
	glm::vec4 sunlightDirection; //w for sun power
	glm::vec4 sunlightColor;
};

struct FrameDescriptors
{
	AllocatedBuffer cameraBuffer;
	VkDescriptorSet globalDescriptor;

	AllocatedBuffer objectBuffer;
	VkDescriptorSet objectDescriptor;
};

constexpr unsigned int FRAME_OVERLAP_X = 1; // 2

class DrawObjectsPass : public Pass
{
public:
    struct In
    {
        uint32_t outputImageWidth;
        uint32_t outputImageHeight;

        VkFormat colorImageFormat;
        VkFormat depthImageFormat;

        GPUSceneData _sceneParameters;

        class VulkanEngine* engine = nullptr;
    };

    FrameDescriptors _frames[FRAME_OVERLAP_X];

	VkDescriptorPool _descriptorPool;

	VkDescriptorSetLayout _globalSetLayout;
	VkDescriptorSetLayout _objectSetLayout;

	AllocatedBuffer _sceneParameterBuffer;

    ResourceManager resourceManager;

    struct Out
    {
        VkFramebuffer sceneColorFBO;

        AllocatedImage depthImage;
        VkImageView depthImageView;

        AllocatedImage colorImage;
        VkImageView colorImageView;
    };

public:
    In in;
    Out out;

private:
public:
    VkRenderPass renderPass;

    //default array of renderable objects
	std::vector<RenderObject> _renderables;

public:
    void addRenderObject(const RenderObject& renderObject)
	{
		_renderables.emplace_back(renderObject);
	}

private:
	//our draw function
	void draw_objects(MemoryManager& memoryManager, VkCommandBuffer cmd, RenderObject* first, int count);

public:
    void set(const In& in);
    virtual void setup(MemoryManager& memoryManager) override;
    virtual void update(MemoryManager& memoryManager, VkCommandBuffer& cmd) override;
    virtual void destroy(MemoryManager& memoryManager) override;

    void firstSetup(MemoryManager& memoryManager);
    void lastDestroy(MemoryManager& memoryManager);

    VkFramebuffer initTempFrameBuffer(MemoryManager& memoryManager, VkImageView colorImageView, VkImageView depthImageView);

public:
	//loads a shader module from a spir-v file. Returns false if it errors
	static bool load_shader_module(VkDevice _device, const char* filePath, VkShaderModule* outShaderModule);
    void addShaderModule(MemoryManager& memoryManager, const std::string& path);
    void init_pipelines(MemoryManager& memoryManager, VkRenderPass renderPass);
    void init_descriptors(MemoryManager& memoryManager);
};