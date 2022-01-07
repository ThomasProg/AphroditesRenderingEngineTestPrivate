#pragma once

#include <vulkan/vulkan.h>

class Pass
{
public:
    // set automatically
    VkSemaphore semaphoreToWait = VK_NULL_HANDLE;
    VkSemaphore semaphoreToSignal = VK_NULL_HANDLE;

public:
    virtual void setup(class MemoryManager& memoryManager) {}
    virtual void update(class MemoryManager& memoryManager) {}


};