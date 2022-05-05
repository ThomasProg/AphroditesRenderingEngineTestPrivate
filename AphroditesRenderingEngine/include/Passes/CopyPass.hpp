#pragma once

#include "Passes/Pass.hpp"
#include <vk_types.h>

class CopyPass : public Pass
{
public:
    struct In
    {
        int width;
        int height;

        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        VkFilter filter = VK_FILTER_NEAREST;

        VkImage* src = nullptr;
    };

    struct Out
    {
        VkImage* dest = nullptr;
    };

public:
    In in;
    Out out;

public:
    void set(const In& in)
    {
        this->in = in;
    }
    virtual void setup(MemoryManager& memoryManager) override {}
    virtual void update(MemoryManager& memoryManager, VkCommandBuffer& cmd) override;
    virtual void destroy(MemoryManager& memoryManager) override {}
};