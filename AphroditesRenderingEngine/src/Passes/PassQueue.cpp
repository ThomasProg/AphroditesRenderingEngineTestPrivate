#include "Passes/PassQueue.hpp"
#include "MemoryManager.hpp"

#include <cassert>

void PassQueue::init(MemoryManager& memoryManager, VkSemaphore firstSemaphoreToWait, VkSemaphore lastSemaphoreToSignal)
{
    for (int i = 0; i < passes.size() - 1; i++)
    {
        Pass& pass = *passes[i];
        Pass& nextPass = *passes[i+1];

        assert(pass.semaphoreToSignal == VK_NULL_HANDLE && pass.semaphoreToSignal == VK_NULL_HANDLE);
        memoryManager.createSemaphore(pass.semaphoreToSignal);
        nextPass.semaphoreToWait = pass.semaphoreToSignal;
    }

    setEndSemaphores(firstSemaphoreToWait, lastSemaphoreToSignal);
}

void PassQueue::setEndSemaphores(VkSemaphore firstSemaphoreToWait, VkSemaphore lastSemaphoreToSignal)
{
    passes.front()->semaphoreToWait = firstSemaphoreToWait;
    passes.back()->semaphoreToSignal = lastSemaphoreToSignal;
}

void PassQueue::update(class MemoryManager& memoryManager)
{
    for (auto& pass : passes)
    {
        pass->update(memoryManager);
    }
}

void PassQueue::destroy(class MemoryManager& memoryManager)
{
    for (int i = 0; i < passes.size() - 1; i++)
    {
        Pass& pass = *passes[i];
        Pass& nextPass = *passes[i+1];

        memoryManager.destroySemaphore(pass.semaphoreToSignal);
        pass.semaphoreToSignal = VK_NULL_HANDLE;
        nextPass.semaphoreToWait = VK_NULL_HANDLE;
    } 
}
