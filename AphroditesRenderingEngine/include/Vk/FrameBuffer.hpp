#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>

class FrameBuffer
{
public:
    VkFramebuffer frameBuffer;

    void init(VkDevice device, VkImageView colorBuffer, VkImageView depthBuffer, VkRenderPass renderPass, VkExtent2D size) // TODO : Stencil Buffer ? 
    {
        std::array<VkImageView, 2> attachments = 
        {
            colorBuffer,
            depthBuffer
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = size.width;
        framebufferInfo.height = size.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &frameBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

    operator VkFramebuffer()
    {
        return frameBuffer;
    }
};