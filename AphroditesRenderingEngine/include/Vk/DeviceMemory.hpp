#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "LogicalDevice.hpp"

// Memory allocated on a device (gpu)
class DeviceMemory
{
public:
    VkDeviceMemory memory;

    DeviceMemory() = default;
    inline DeviceMemory(VkDevice device, VkDeviceSize memorySize, uint32_t memoryTypeIndex)
    {
        init(device, memorySize, memoryTypeIndex);
    }

    void init(VkDevice device, VkDeviceSize memorySize, uint32_t memoryTypeIndex)
    {
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memorySize;
        allocInfo.memoryTypeIndex = memoryTypeIndex;

        if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to allocate buffer memory!");
        }
    }

    void free(VkDevice device)
    {
        vkFreeMemory(device, memory, nullptr);
    }

    void sendData(LogicalDevice& device, void* toSend, size_t bufferSize)
    {
        void* data;
        vkMapMemory(device, memory, 0, bufferSize, 0, &data);
        memcpy(data, toSend, bufferSize);
        vkUnmapMemory(device, memory);
    }
};
