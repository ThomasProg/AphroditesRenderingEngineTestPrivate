#pragma once

#include "Passes/Pass.hpp"
#include <vk_types.h>

class ComputePass : public Pass
{
public:
    struct In
    {
        class ComputeMaterial* mat = nullptr;
        int width;
        int height;
        VkFormat colorImageFormat;
        VkFormat viewFormat;
    };

    struct Out
    {
        AllocatedImage outColorImage;
        VkImageView outColorImageView;
    };

public:
    In in;
    Out out;

public:
    void set(const In& in);
    virtual void setup(MemoryManager& memoryManager) override;
    virtual void update(MemoryManager& memoryManager, VkCommandBuffer& cmd) override;
    virtual void destroy(MemoryManager& memoryManager) override;
};