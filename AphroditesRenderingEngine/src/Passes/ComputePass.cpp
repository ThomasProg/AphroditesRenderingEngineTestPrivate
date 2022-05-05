#include "Passes/ComputePass.hpp"
#include "Material.hpp"
#include "MemoryManager.hpp"

#include "Test/vk_initializers.h"

void ComputePass::set(const In& in)
{
    this->in = in;
}

void ComputePass::setup(MemoryManager& memoryManager) 
{
    memoryManager.createAllocatedImage2D(out.outColorImage, in.colorImageFormat, 
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, in.width, in.height);

    VkImageViewCreateInfo dColorview_info = vkinit::imageview_create_info(in.viewFormat, out.outColorImage._image, VK_IMAGE_ASPECT_COLOR_BIT);
    VK_CHECK(vkCreateImageView(memoryManager._device, &dColorview_info, nullptr, &out.outColorImageView));

    std::cout << "compute pass udpate << " << in.width << " / " << in.height << std::endl;
}


void ComputePass::update(MemoryManager& memoryManager, VkCommandBuffer& cmd)
{
    in.mat->bind(cmd);
    vkCmdDispatch(cmd, (in.width / 16) + 1, (in.height / 16) + 1, 1);
}

void ComputePass::destroy(MemoryManager& memoryManager) 
{
    vkDestroyImageView(memoryManager._device, out.outColorImageView, nullptr);
    memoryManager.destroyAllocatedImage(out.outColorImage);
    // vmaDestroyImage(memoryManager._allocator, out.outColorImage._image, out.outColorImage._allocation); 
}