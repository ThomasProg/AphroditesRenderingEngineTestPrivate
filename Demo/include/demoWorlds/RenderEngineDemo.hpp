#pragma once

#include "Test/vk_engine.h"

#include "Materials/Materials.hpp"

#include "Passes/DrawObjectsPass.hpp"
#include "Passes/ComputePass.hpp"

struct ComputeShaderDemo
{
    MemoryManager* memoryManager = nullptr;
    VkQueue queue;								// Separate queue for compute commands (queue family may differ from the one used for graphics)
    int width, height;
    EdgeDetectMaterial mat;

    VkDescriptorPool descriptorPool;

    VkDescriptorImageInfo inImageDescriptor;
    VkDescriptorImageInfo outImageDescriptor;

	void buildComputeCommandBuffer(VkCommandBuffer cmd);

public:
	void createFullSwapchain(std::unordered_map<std::string, VkShaderModule>& shaderModules);
    void destroyFullSwapchain();
};

class RenderEngineDemo : public VulkanEngine
{
    using Super = VulkanEngine;

	virtual void recordRenderPasses(VkFramebuffer targetFramebuffer);

    void recordBlur(VkCommandBuffer cmd, VkFramebuffer targetFramebuffer);

    VkFramebuffer initTempFrameBuffer(VkImageView colorImageView, VkImageView depthImageView);

    DrawObjectsPass drawObjectsPass;
    ComputePass computePass;

    struct 
    {
        VkRenderPass renderPass;

        VkFramebuffer sceneColorFBO;

        AllocatedImage depthImage;
        VkImageView depthImageView;

        AllocatedImage colorImage;
        VkImageView colorImageView;

        AllocatedImage outColorImage;
        VkImageView outColorImageView;

        VkCommandPool _commandPool;
        VkCommandBuffer _mainCommandBuffer;

        VkFence fence;

        VkFence computeFence;
    } tempPass;

    ComputeShaderDemo computeShader;

    uint32_t swapchainImageIndex;

    AllocatedImage createColorImage(MemoryManager& memoryManager, VkImageUsageFlags usageFlags);
    AllocatedImage createDepthImage();

public:
	virtual void createFullSwapchain() override;
    virtual void destroyFullSwapchain() override;

    virtual void draw() override;
};