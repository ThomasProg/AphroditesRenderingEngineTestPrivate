#include "demoWorlds/RenderEngineDemo.hpp"

#include "vk_initializers.h"
#include "Window.hpp"

#include <thread>
void RenderEngineDemo::draw()
{
	//wait until the gpu has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(context->memoryManager->_device, 1, &get_current_frame()._renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(context->memoryManager->_device, 1, &get_current_frame()._renderFence));

	//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(get_current_frame()._mainCommandBuffer, 0));

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

    recordRenderPasses();

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

void RenderEngineDemo::recordRenderPasses()
{
    VkCommandBuffer& cmd = get_current_frame()._mainCommandBuffer;

	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    context->memoryManager->changeImageLayoutFullBarrier(cmd, drawObjectsPass.out.colorImage._image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    drawObjectsPass.update(*context->memoryManager, cmd);
    context->memoryManager->changeImageLayoutFullBarrier(cmd, computePass.out.outColorImage._image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    computePass.update(*context->memoryManager, cmd);
    context->memoryManager->changeImageLayoutFullBarrier(cmd, computePass.out.outColorImage._image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    context->memoryManager->changeImageLayoutFullBarrier(cmd, swapChainData._colorImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyPass.out.dest = &swapChainData._colorImages[swapchainImageIndex];
    copyPass.update(*context->memoryManager, cmd);

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


void RenderEngineDemo::createFullSwapchain() 
{
	swapChainData = init_swapchain(_surface, context->memoryManager->_chosenGPU, _window, context->memoryManager->_device, context->memoryManager->_allocator); //

    // renderPass = init_default_renderpass(swapChainData._colorImageFormat /* VK_FORMAT_B8G8R8A8_UNORM*/, swapChainData._depthImageFormat, VK_IMAGE_LAYOUT_GENERAL);

	init_commands(); //

    // Super::createFullSwapchain();

    {
        VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(context->memoryManager->queueFamilies.graphicsFamily.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        VK_CHECK(vkCreateCommandPool(context->memoryManager->_device, &commandPoolInfo, nullptr, &tempPass._commandPool));
        VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(tempPass._commandPool, 1);
        VK_CHECK(vkAllocateCommandBuffers(context->memoryManager->_device, &cmdAllocInfo, &tempPass._mainCommandBuffer));
    }

    {
        DrawObjectsPass::In in;
        in.engine = this;
        in.outputImageWidth = (uint32_t) _window->width;
        in.outputImageHeight = (uint32_t) _window->height;
        in.colorImageFormat = swapChainData._colorImageFormat;
        in.depthImageFormat = swapChainData._depthImageFormat;
        drawObjectsPass.set(in);
        drawObjectsPass.setup(*context->memoryManager);
    }

    {
        ComputePass::In in;
        in.width = (uint32_t) _window->width;
        in.height = (uint32_t) _window->height;
        in.mat = &mat;
        in.colorImageFormat = swapChainData._colorImageFormat;
        VkFormat usedFormat = swapChainData._colorImageFormat;//VK_FORMAT_B8G8R8A8_UNORM; /*swapChainData._colorImageFormat*/
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(context->memoryManager->_chosenGPU, usedFormat, &formatProperties);
        assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);
        in.viewFormat = usedFormat;

        computePass.set(in);
        computePass.setup(*context->memoryManager);
    }

    {
        CopyPass::In in;
        in.width = (uint32_t) _window->width;
        in.height = (uint32_t) _window->height;
        in.src = &computePass.out.outColorImage._image;
        copyPass.set(in);
        copyPass.setup(*context->memoryManager);
    }

    // computeShader.createFullSwapchain(resourceManager.shaderModules);
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
    vkCreateDescriptorPool(context->memoryManager->_device, &pCreateInfo, nullptr, &descriptorPool);

    VkDescriptorImageInfo& inImageDescriptor = mat.inImageDescriptor;
    inImageDescriptor.imageView = drawObjectsPass.out.colorImageView;
    inImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;// VK_IMAGE_LAYOUT_UNDEFINED;

    VkDescriptorImageInfo& outImageDescriptor = mat.outImageDescriptor;
    outImageDescriptor.imageView = computePass.out.outColorImageView;
    outImageDescriptor.imageLayout =  VK_IMAGE_LAYOUT_GENERAL; // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    mat.descriptorPool = descriptorPool;
    mat.shaderModules = &drawObjectsPass.resourceManager.shaderModules;

    mat.init(*context->memoryManager);

	drawObjectsPass.init_pipelines(*context->memoryManager, drawObjectsPass.renderPass); 
}

void RenderEngineDemo::destroyFullSwapchain() 
{
    Super::destroyFullSwapchain();

    // computeShader.destroyFullSwapchain();
    mat.destroy();
    vkDestroyDescriptorPool(context->memoryManager->_device, descriptorPool, nullptr);

    // Destroy pool and command buffers inside
    vkDestroyCommandPool(context->memoryManager->_device, tempPass._commandPool, nullptr);

    copyPass.destroy(*context->memoryManager);
    computePass.destroy(*context->memoryManager);
    drawObjectsPass.destroy(*context->memoryManager);
}


void RenderEngineDemo::firstSetup(MemoryManager& memoryManager) 
{
    drawObjectsPass.firstSetup(memoryManager);
}

void RenderEngineDemo::lastDestroy(MemoryManager& memoryManager) 
{
    drawObjectsPass.lastDestroy(memoryManager);
}
