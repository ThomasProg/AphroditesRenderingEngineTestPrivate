#include "Passes/DrawObjectsPass.hpp"

#include "vk_initializers.h"
#include "MemoryManager.hpp"

#include "Test/vk_engine.h"

void DrawObjectsPass::set(const In& in)
{
    this->in = in;
}

AllocatedImage DrawObjectsPass::createDepthImage(MemoryManager& memoryManager)
{
    //depth image size will match the window
    VkExtent3D depthImageExtent = {
        in.outputImageWidth,
        in.outputImageHeight,
        1
    };

    // //hardcoding the depth format to 32 bit float
    // swapChainData._depthImageFormat = VK_FORMAT_D32_SFLOAT;

    //the depth image will be a image with the format we selected and Depth Attachment usage flag
    VkImageCreateInfo dimg_info = vkinit::image_create_info(in.depthImageFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

    //for the depth image, we want to allocate it from gpu local memory
    VmaAllocationCreateInfo dimg_allocinfo = {};
    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //allocate and create the image
    AllocatedImage depthImage;
    vmaCreateImage(memoryManager._allocator, &dimg_info, &dimg_allocinfo, &depthImage._image, &depthImage._allocation, nullptr);

    return depthImage;
}

AllocatedImage DrawObjectsPass::createColorImage(MemoryManager& memoryManager, VkImageUsageFlags usageFlags)
{
    // color image size will match the window
    VkExtent3D colorImageExtent = {
        in.outputImageWidth,
        in.outputImageHeight,
        1
    };

    VkImageCreateInfo dimg_info = vkinit::image_create_info(in.colorImageFormat /* VK_FORMAT_B8G8R8A8_UNORM */, usageFlags, colorImageExtent);

    //for the color image, we want to allocate it from gpu local memory
    VmaAllocationCreateInfo dimg_allocinfo = {};
    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //allocate and create the image
    AllocatedImage colorImage;
    vmaCreateImage(memoryManager._allocator, &dimg_info, &dimg_allocinfo, &colorImage._image, &colorImage._allocation, nullptr);

    return colorImage;
}


void DrawObjectsPass::setup(MemoryManager& memoryManager) 
{
    renderPass = memoryManager.init_default_renderpass(in.colorImageFormat /* VK_FORMAT_B8G8R8A8_UNORM*/, in.depthImageFormat, VK_IMAGE_LAYOUT_GENERAL);

	// Create Swapchain's depth buffer
    out.depthImage = createDepthImage(memoryManager);

    // build a image-view for the depth image to use for rendering
    VkImageViewCreateInfo dDepthview_info = vkinit::imageview_create_info(in.depthImageFormat, out.depthImage._image, VK_IMAGE_ASPECT_DEPTH_BIT);
    VK_CHECK(vkCreateImageView(memoryManager._device, &dDepthview_info, nullptr, &out.depthImageView));

    VkFormatProperties formatProperties;

    VkFormat usedFormat = VK_FORMAT_B8G8R8A8_UNORM; /*swapChainData._colorImageFormat*/

    // Get device properties for the requested texture format
    vkGetPhysicalDeviceFormatProperties(memoryManager._chosenGPU, usedFormat, &formatProperties);
    // Check if requested image format supports image storage operations
    assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);

    // src
    {
        out.colorImage = createColorImage(memoryManager, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT/* VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT*/);
        VkImageViewCreateInfo dColorview_info = vkinit::imageview_create_info(usedFormat, out.colorImage._image, VK_IMAGE_ASPECT_COLOR_BIT);
        VK_CHECK(vkCreateImageView(memoryManager._device, &dColorview_info, nullptr, &out.colorImageView));
    }

    out.sceneColorFBO = initTempFrameBuffer(memoryManager, out.colorImageView, out.depthImageView);

}

VkFramebuffer DrawObjectsPass::initTempFrameBuffer(class MemoryManager& memoryManager, VkImageView colorImageView, VkImageView depthImageView)
{
	//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	VkFramebufferCreateInfo fb_info = vkinit::framebuffer_create_info(renderPass, {in.outputImageWidth, in.outputImageHeight});

    VkImageView attachments[2];
    attachments[0] = colorImageView;
    attachments[1] = depthImageView;

    fb_info.pAttachments = attachments;
    fb_info.attachmentCount = 2;

    VkFramebuffer fbo;
    VK_CHECK(vkCreateFramebuffer(memoryManager._device, &fb_info, nullptr, &fbo));
    return fbo;
}

void DrawObjectsPass::update(class MemoryManager& memoryManager, VkCommandBuffer& cmd)
{
    in.engine->recordColorRenderPass(cmd, renderPass, out.sceneColorFBO);
}



void DrawObjectsPass::destroy(class MemoryManager& memoryManager)
{
    vkDestroyFramebuffer(memoryManager._device, out.sceneColorFBO, nullptr);

    vkDestroyImageView(memoryManager._device, out.colorImageView, nullptr);
    vmaDestroyImage(memoryManager._allocator, out.colorImage._image, out.colorImage._allocation); 
    
    vkDestroyImageView(memoryManager._device, out.depthImageView, nullptr);
    vmaDestroyImage(memoryManager._allocator, out.depthImage._image, out.depthImage._allocation);

    vkDestroyRenderPass(memoryManager._device, renderPass, nullptr);
}