#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vk
{

// TODO : Custom allocators ?
class Buffer
{
public:
    VkBuffer buffer;

public:
    Buffer() = default;
    inline Buffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage)
    {
        init(device, size, usage);
    }

    void init(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage) 
    {       
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }   
        std::cout << "id : " << buffer << std::endl;
    }

    // void destroy(VkDevice device)
    // {
    //     std::cout << "destroying : " << buffer << std::endl;
    //     vkDestroyBuffer(device, buffer, nullptr);
    // }
};

}
