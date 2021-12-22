#pragma once

#include "Mesh.hpp"
#include "Vk/Buffer.hpp"
#include "Vk/DeviceMemory.hpp"

class BufferManager
{
public:

    // DeviceMemory* vertexBufferMemory;

    DeviceMemory* findLocalMemory(VkBuffer buffer, LogicalDevice& device, VkPhysicalDevice physicalDevice)
    {
        VkMemoryRequirements vertexMemRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &vertexMemRequirements);
        return new DeviceMemory(device, vertexMemRequirements.size, 
            PhysicalDevice::findMemoryType(physicalDevice, vertexMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
    }

}; 