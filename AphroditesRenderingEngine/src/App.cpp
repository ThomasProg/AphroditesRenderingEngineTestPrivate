#include "VulkanSubsystem.hpp"
#include "Texture.hpp"
#include "Vk/Buffer.hpp"

void HelloTriangleApplication::createTextureImage(Texture& tex) {
    VkDeviceSize imageSize = tex.getWidth() * tex.getHeight() * 4;

    vk::Buffer stagingBuffer(presenter.device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(presenter.device, stagingBuffer.buffer, &memRequirements);
    DeviceMemory stagingBufferMemory(presenter.device, memRequirements.size, 
        PhysicalDevice::findMemoryType(presenter.physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
    vkBindBufferMemory(presenter.device, stagingBuffer.buffer, stagingBufferMemory.memory, 0);

    void* data;
    vkMapMemory(presenter.device, stagingBufferMemory.memory, 0, imageSize, 0, &data);
    memcpy(data, tex.getPixels(), static_cast<size_t>(imageSize));
    vkUnmapMemory(presenter.device, stagingBufferMemory.memory);

    createImage(tex.getWidth(), tex.getHeight(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer.buffer, textureImage, static_cast<uint32_t>(tex.getWidth()), static_cast<uint32_t>(tex.getHeight()));
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


    stagingBuffer.destroy(presenter.device);
    stagingBufferMemory.free(presenter.device);
}
