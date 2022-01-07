#pragma once

#include <memory>
#include <vector>
#include "Passes/Pass.hpp"

class PassQueue
{
public:
    std::vector<std::unique_ptr<Pass>> passes;

    void init(class MemoryManager& memoryManager, VkSemaphore firstSemaphoreToWait, VkSemaphore lastSemaphoreToSignal);
    void update(class MemoryManager& memoryManager);
    void destroy(class MemoryManager& memoryManager);

    void setEndSemaphores(VkSemaphore firstSemaphoreToWait, VkSemaphore lastSemaphoreToSignal);
};