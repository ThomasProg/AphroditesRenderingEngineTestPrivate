#pragma once

#include <vulkan/vulkan.h>

class MemoryManager;

class Pass
{
public:
    // set automatically
    VkSemaphore semaphoreToWait = VK_NULL_HANDLE;
    VkSemaphore semaphoreToSignal = VK_NULL_HANDLE;

public:
    virtual void setup(MemoryManager& memoryManager) = 0;
    virtual void update(MemoryManager& memoryManager, VkCommandBuffer& cmd) = 0;
    virtual void destroy(MemoryManager& memoryManager) = 0;

};