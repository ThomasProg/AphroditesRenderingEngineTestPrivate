#include "Passes/CopyPass.hpp"




void CopyPass::update(MemoryManager& memoryManager, VkCommandBuffer& cmd) 
{
    VkOffset3D blitSize;
    blitSize.x = in.width;
    blitSize.y = in.height;
    blitSize.z = 1;
    VkImageBlit imageBlitRegion{};
    imageBlitRegion.srcSubresource.aspectMask = in.aspectMask;
    imageBlitRegion.srcSubresource.layerCount = 1;
    imageBlitRegion.srcOffsets[1] = blitSize;
    imageBlitRegion.dstSubresource.aspectMask = in.aspectMask;
    imageBlitRegion.dstSubresource.layerCount = 1;
    imageBlitRegion.dstOffsets[1] = blitSize;

    vkCmdBlitImage(cmd, 
        *in.src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
        *out.dest, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &imageBlitRegion,
        in.filter);
}