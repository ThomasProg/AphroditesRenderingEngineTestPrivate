#pragma once

#include "Vk/Buffer.hpp"
#include "Vk/DeviceMemory.hpp"

class BoundBuffer
{
private:
public:
    DeviceMemory stagingBufferMemory;

public:
    vk::Buffer stagingBuffer;

    void initAsTransferSrc(VkDevice device, VkPhysicalDevice physicalDevice, size_t bufferSize)
    {
        stagingBuffer.init(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, stagingBuffer.buffer, &memRequirements);
        stagingBufferMemory.init(device, memRequirements.size, 
            PhysicalDevice::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
        vkBindBufferMemory(device, stagingBuffer.buffer, stagingBufferMemory.memory, 0);
    }

    void initAsVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, size_t bufferSize)
    {
        stagingBuffer.init(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        VkMemoryRequirements vertexMemRequirements;
        vkGetBufferMemoryRequirements(device, stagingBuffer.buffer, &vertexMemRequirements);
        stagingBufferMemory.init(device, vertexMemRequirements.size, 
            PhysicalDevice::findMemoryType(physicalDevice, vertexMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
        vkBindBufferMemory(device, stagingBuffer.buffer, stagingBufferMemory.memory, 0);
    }

    void initAsIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, size_t bufferSize)
    {
        stagingBuffer.init(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        VkMemoryRequirements indexMemRequirements;
        vkGetBufferMemoryRequirements(device, stagingBuffer.buffer, &indexMemRequirements);
        stagingBufferMemory.init(device, indexMemRequirements.size, 
            PhysicalDevice::findMemoryType(physicalDevice, indexMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
        vkBindBufferMemory(device, stagingBuffer.buffer, stagingBufferMemory.memory, 0);
    }

    inline void sendData(LogicalDevice& device, void* toSend, size_t bufferSize)
    {
        stagingBufferMemory.sendData(device, toSend, bufferSize);
    }

    void destroy(VkDevice device)
    {
        stagingBuffer.destroy(device);
        stagingBufferMemory.free(device);
    }
};