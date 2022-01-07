#pragma once

#include "Passes/Pass.hpp"
#include <vk_types.h>

class MemoryManager;

class DrawObjectsPass : public Pass
{
public:
    struct In
    {
        uint32_t outputImageWidth;
        uint32_t outputImageHeight;

        VkFormat colorImageFormat;
        VkFormat depthImageFormat;

        class VulkanEngine* engine = nullptr;
    };

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
    VkRenderPass renderPass;

public:
    void set(const In& in);
    virtual void setup(MemoryManager& memoryManager) override;
    virtual void update(MemoryManager& memoryManager, VkCommandBuffer& cmd);

    virtual void destroy(MemoryManager& memoryManager);



    AllocatedImage createDepthImage(MemoryManager& memoryManager);
    AllocatedImage createColorImage(MemoryManager& memoryManager, VkImageUsageFlags usageFlags);
    VkFramebuffer initTempFrameBuffer(MemoryManager& memoryManager, VkImageView colorImageView, VkImageView depthImageView);
};