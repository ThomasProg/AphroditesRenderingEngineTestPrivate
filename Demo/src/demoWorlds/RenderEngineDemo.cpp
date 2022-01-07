#include "demoWorlds/RenderEngineDemo.hpp"

#include "vk_initializers.h"
#include "Window.hpp"


void ComputeShaderDemo::buildComputeCommandBuffer(VkCommandBuffer cmd)
{
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, mat.pipeline);
    mat.bind(*memoryManager, cmd);
    vkCmdDispatch(cmd, width / 16, height / 16, 1);
}

void ComputeShaderDemo::createFullSwapchain(std::unordered_map<std::string, VkShaderModule>& shaderModules)
{
    std::vector<VkDescriptorPoolSize> sizes =
    {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2 }
    };
    VkDescriptorPoolCreateInfo pCreateInfo = {};
    pCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pCreateInfo.flags = 0;
    pCreateInfo.maxSets = 3;
    pCreateInfo.poolSizeCount = (uint32_t)sizes.size();
    pCreateInfo.pPoolSizes = sizes.data();
    vkCreateDescriptorPool(memoryManager->_device, &pCreateInfo, nullptr, &descriptorPool);

    mat.inImageDescriptor = inImageDescriptor;
    mat.outImageDescriptor = outImageDescriptor;

    mat.createUniforms(*memoryManager, descriptorPool);
    mat.initUniforms(*memoryManager);
    mat.makePipelineLayout(*memoryManager);
    mat.makePipeline(*memoryManager, shaderModules);

    // mat.bindUniforms();
}

void ComputeShaderDemo::destroyFullSwapchain()
{
    mat.destroyPipeline(*memoryManager);
    mat.destroyUniforms(*memoryManager);

    vkDestroyDescriptorPool(memoryManager->_device, descriptorPool, nullptr);
}


void RenderEngineDemo::recordBlur(VkCommandBuffer cmd, VkFramebuffer targetFramebuffer)
{

}

AllocatedImage RenderEngineDemo::createColorImage(MemoryManager& memoryManager, VkImageUsageFlags usageFlags)
{
    // color image size will match the window
    VkExtent3D colorImageExtent = {
        (uint32_t) _window->width,
        (uint32_t) _window->height,
        1
    };

    VkImageCreateInfo dimg_info = vkinit::image_create_info(swapChainData._colorImageFormat /* VK_FORMAT_B8G8R8A8_UNORM */, usageFlags, colorImageExtent);

    //for the color image, we want to allocate it from gpu local memory
    VmaAllocationCreateInfo dimg_allocinfo = {};
    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //allocate and create the image
    AllocatedImage colorImage;
    vmaCreateImage(memoryManager._allocator, &dimg_info, &dimg_allocinfo, &colorImage._image, &colorImage._allocation, nullptr);

    return colorImage;
}

AllocatedImage RenderEngineDemo::createDepthImage()
{
    //depth image size will match the window
    VkExtent3D depthImageExtent = {
        (uint32_t) _window->width,
        (uint32_t) _window->height,
        1
    };

    // //hardcoding the depth format to 32 bit float
    // swapChainData._depthImageFormat = VK_FORMAT_D32_SFLOAT;

    //the depth image will be a image with the format we selected and Depth Attachment usage flag
    VkImageCreateInfo dimg_info = vkinit::image_create_info(swapChainData._depthImageFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

    //for the depth image, we want to allocate it from gpu local memory
    VmaAllocationCreateInfo dimg_allocinfo = {};
    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //allocate and create the image
    AllocatedImage depthImage;
    vmaCreateImage(context->memoryManager->_allocator, &dimg_info, &dimg_allocinfo, &depthImage._image, &depthImage._allocation, nullptr);

    return depthImage;
}

#include <thread>
void RenderEngineDemo::draw()
{
	//wait until the gpu has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(context->memoryManager->_device, 1, &get_current_frame()._renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(context->memoryManager->_device, 1, &get_current_frame()._renderFence));

	//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(get_current_frame()._mainCommandBuffer, 0));

        // {
        //     VK_CHECK(vkWaitForFences(context->memoryManager->_device, 1, &tempPass.fence, true, 1000000000));
        //     VK_CHECK(vkResetFences(context->memoryManager->_device, 1, &tempPass.fence));
        //     VK_CHECK(vkResetCommandBuffer(tempPass._mainCommandBuffer, 0));
        // }

	// framebuffer / image to use to render to viewport
	// uint32_t swapchainImageIndex;

	//request image from the swapchain
	VkResult result = vkAcquireNextImageKHR(context->memoryManager->_device, swapChainData._swapchain, 1000000000, get_current_frame()._presentSemaphore, nullptr, &swapchainImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) 
	{
		recreateSwapchain();
		return;
	} 
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

    // ONLY WORKS WITH 1 FRAME
    // TODO : Multiple PassQueue
    // passQueue.setEndSemaphores(get_current_frame()._presentSemaphore, get_current_frame()._renderSemaphore);

    recordRenderPasses(_framebuffers[swapchainImageIndex]);


    VkPresentInfoKHR presentInfo = vkinit::present_info();

	presentInfo.pSwapchains = &swapChainData._swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &get_current_frame()._renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	result = vkQueuePresentKHR(context->memoryManager->_graphicsQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) 
	{
		recreateSwapchain();
	}
	else if (result != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	//increase the number of frames drawn
	_frameNumber++;
}

void RenderEngineDemo::recordRenderPasses(VkFramebuffer targetFramebuffer)
{
    VkCommandBuffer& cmd = get_current_frame()._mainCommandBuffer;

	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    {
        context->memoryManager->changeImageLayoutFullBarrier(cmd, tempPass.outColorImage._image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        context->memoryManager->changeImageLayoutFullBarrier(cmd, drawObjectsPass.out.colorImage._image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        drawObjectsPass.update(*context->memoryManager, cmd);
    }

    {
        computeShader.buildComputeCommandBuffer(get_current_frame()._mainCommandBuffer);
    }
    
    {
        context->memoryManager->changeImageLayoutFullBarrier(cmd, tempPass.outColorImage._image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    }

    {
        context->memoryManager->changeImageLayoutFullBarrier(cmd, swapChainData._colorImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    }

    {
        // Define the region to blit (we will blit the whole swapchain image)
        VkOffset3D blitSize;
        blitSize.x = _window->width;
        blitSize.y = _window->height;
        blitSize.z = 1;
        VkImageBlit imageBlitRegion{};
        imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.srcSubresource.layerCount = 1;
        imageBlitRegion.srcOffsets[1] = blitSize;
        imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.dstSubresource.layerCount = 1;
        imageBlitRegion.dstOffsets[1] = blitSize;

        vkCmdBlitImage(cmd, 
            tempPass.outColorImage._image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
            swapChainData._colorImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageBlitRegion,
            VK_FILTER_NEAREST);
    }

    context->memoryManager->changeImageLayoutFullBarrier(cmd, swapChainData._colorImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(cmd));

    VkSubmitInfo submit = vkinit::submit_info(&cmd);
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    submit.pWaitDstStageMask = &waitStage;

    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &get_current_frame()._presentSemaphore;

    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &get_current_frame()._renderSemaphore;


    //submit command buffer to the queue and execute it.
    // _renderFence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit(context->memoryManager->_graphicsQueue, 1, &submit, get_current_frame()._renderFence));
}


VkFramebuffer RenderEngineDemo::initTempFrameBuffer(VkImageView colorImageView, VkImageView depthImageView)
{
	//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	VkFramebufferCreateInfo fb_info = vkinit::framebuffer_create_info(tempPass.renderPass, {(uint32_t)_window->width, (uint32_t)_window->height});

    VkImageView attachments[2];
    attachments[0] = colorImageView;
    attachments[1] = depthImageView;

    fb_info.pAttachments = attachments;
    fb_info.attachmentCount = 2;

    VkFramebuffer fbo;
    VK_CHECK(vkCreateFramebuffer(context->memoryManager->_device, &fb_info, nullptr, &fbo));
    return fbo;
}

void RenderEngineDemo::createFullSwapchain() 
{
    Super::createFullSwapchain();

    // TODO OUTSIDE
    VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
    VK_CHECK(vkCreateFence(context->memoryManager->_device, &fenceCreateInfo, nullptr, &tempPass.fence));
    VK_CHECK(vkCreateFence(context->memoryManager->_device, &fenceCreateInfo, nullptr, &tempPass.computeFence));

    tempPass.renderPass = init_default_renderpass(swapChainData._colorImageFormat /* VK_FORMAT_B8G8R8A8_UNORM*/, swapChainData._depthImageFormat, VK_IMAGE_LAYOUT_GENERAL);

    {
        VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(context->memoryManager->queueFamilies.graphicsFamily.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        VK_CHECK(vkCreateCommandPool(context->memoryManager->_device, &commandPoolInfo, nullptr, &tempPass._commandPool));
        VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(tempPass._commandPool, 1);
        VK_CHECK(vkAllocateCommandBuffers(context->memoryManager->_device, &cmdAllocInfo, &tempPass._mainCommandBuffer));
    }

	// Create Swapchain's depth buffer
    tempPass.depthImage = createDepthImage();

    // build a image-view for the depth image to use for rendering
    VkImageViewCreateInfo dDepthview_info = vkinit::imageview_create_info(swapChainData._depthImageFormat, tempPass.depthImage._image, VK_IMAGE_ASPECT_DEPTH_BIT);
    VK_CHECK(vkCreateImageView(context->memoryManager->_device, &dDepthview_info, nullptr, &tempPass.depthImageView));

    VkFormatProperties formatProperties;

    VkFormat usedFormat = VK_FORMAT_B8G8R8A8_UNORM; /*swapChainData._colorImageFormat*/

    // Get device properties for the requested texture format
    vkGetPhysicalDeviceFormatProperties(context->memoryManager->_chosenGPU, usedFormat, &formatProperties);
    // Check if requested image format supports image storage operations
    assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);

    // src
    {
        tempPass.colorImage = createColorImage(*context->memoryManager, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT/* VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT*/);
        VkImageViewCreateInfo dColorview_info = vkinit::imageview_create_info(usedFormat, tempPass.colorImage._image, VK_IMAGE_ASPECT_COLOR_BIT);
        VK_CHECK(vkCreateImageView(context->memoryManager->_device, &dColorview_info, nullptr, &tempPass.colorImageView));
    }

    // target
    {
        tempPass.outColorImage = createColorImage(*context->memoryManager, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT /*VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT*/);

        VkImageViewCreateInfo dColorview_info = vkinit::imageview_create_info(usedFormat, tempPass.outColorImage._image, VK_IMAGE_ASPECT_COLOR_BIT);;
        VK_CHECK(vkCreateImageView(context->memoryManager->_device, &dColorview_info, nullptr, &tempPass.outColorImageView));
    }

    tempPass.sceneColorFBO = initTempFrameBuffer(tempPass.colorImageView, tempPass.depthImageView);


    computeShader.queue = context->memoryManager->_graphicsQueue;
    computeShader.memoryManager = context->memoryManager;
    computeShader.width = _window->width;
    computeShader.height = _window->height;

    DrawObjectsPass::In in;
    in.engine = this;
    in.outputImageWidth = (uint32_t) _window->width;
    in.outputImageHeight = (uint32_t) _window->height;
    in.colorImageFormat = swapChainData._colorImageFormat;
    in.depthImageFormat = swapChainData._depthImageFormat;
    drawObjectsPass.set(in);
    drawObjectsPass.setup(*context->memoryManager);

	VkDescriptorImageInfo& inImageDescriptor = computeShader.inImageDescriptor;
    inImageDescriptor.imageView = drawObjectsPass.out.colorImageView;
    inImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;// VK_IMAGE_LAYOUT_UNDEFINED;

	VkDescriptorImageInfo& outImageDescriptor = computeShader.outImageDescriptor;;
    outImageDescriptor.imageView = tempPass.outColorImageView;
    outImageDescriptor.imageLayout =  VK_IMAGE_LAYOUT_GENERAL; // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    computeShader.createFullSwapchain(resourceManager.shaderModules);
}

void RenderEngineDemo::destroyFullSwapchain() 
{
    Super::destroyFullSwapchain();

    computeShader.destroyFullSwapchain();

    // TODO OUTSIDE
    vkDestroyFence(context->memoryManager->_device, tempPass.fence, nullptr);
    vkDestroyFence(context->memoryManager->_device, tempPass.computeFence, nullptr);

    // Destroy pool and command buffers inside
    vkDestroyCommandPool(context->memoryManager->_device, tempPass._commandPool, nullptr);

    vkDestroyFramebuffer(context->memoryManager->_device, tempPass.sceneColorFBO, nullptr);

    vkDestroyImageView(context->memoryManager->_device, tempPass.colorImageView, nullptr);
    vmaDestroyImage(context->memoryManager->_allocator, tempPass.colorImage._image, tempPass.colorImage._allocation); 

    vkDestroyImageView(context->memoryManager->_device, tempPass.outColorImageView, nullptr);
    vmaDestroyImage(context->memoryManager->_allocator, tempPass.outColorImage._image, tempPass.outColorImage._allocation); 
    
    // depthImage.destroy();
    vkDestroyImageView(context->memoryManager->_device, tempPass.depthImageView, nullptr);
    vmaDestroyImage(context->memoryManager->_allocator, tempPass.depthImage._image, tempPass.depthImage._allocation);

    vkDestroyRenderPass(context->memoryManager->_device, tempPass.renderPass, nullptr);

    drawObjectsPass.destroy(*context->memoryManager);
}

