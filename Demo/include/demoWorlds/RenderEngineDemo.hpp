#pragma once

#include "Test/vk_engine.h"

#include "Materials/Materials.hpp"

#include "Passes/DrawObjectsPass.hpp"
#include "Passes/ComputePass.hpp"
#include "Passes/CopyPass.hpp"

class RenderEngineDemo : public VulkanEngine
{
    using Super = VulkanEngine;

	virtual void recordRenderPasses();

public:
    DrawObjectsPass drawObjectsPass;
    ComputePass computePass;
    CopyPass copyPass;

private:
    EdgeDetectMaterial mat;

    VkDescriptorPool descriptorPool;

    struct 
    {
        VkCommandPool _commandPool;
        VkCommandBuffer _mainCommandBuffer;

    } tempPass;

    uint32_t swapchainImageIndex;
    
public:
	virtual void createFullSwapchain() override;
    virtual void destroyFullSwapchain() override;

    virtual void firstSetup(MemoryManager& memoryManager) override;
    virtual void lastDestroy(MemoryManager& memoryManager) override;

    virtual void draw() override;
};