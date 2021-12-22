#pragma once

#include "Vk/Buffer.hpp"
#include "Vk/DeviceMemory.hpp"
#include "Presenter.hpp"

class UniformBuffer
{
public:
    
    vk::Buffer buffer;
    DeviceMemory bufferMemory;

    void init(Presenter& presenter, VkDeviceSize bufferSize)
    {
        buffer.init(presenter.device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(presenter.device, buffer.buffer, &memRequirements);
        bufferMemory.init(presenter.device, memRequirements.size, 
        PhysicalDevice::findMemoryType(presenter.physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
        vkBindBufferMemory(presenter.device, buffer.buffer, bufferMemory.memory, 0);
    }

    void destroy(VkDevice device)
    {
        buffer.destroy(device);
        bufferMemory.free(device);
    }
};