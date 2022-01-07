
#include "vk_engine.h"

// #include <VkBootstrap.h>

// #include <SDL.h>
// #include <SDL_vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vk_types.h>
#include <vk_initializers.h>

// #include "VkBootstrap.h"

#include <iostream>
#include <fstream>

#include "Window.hpp"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"


constexpr bool bUseValidationLayers = true;

void VulkanEngine::recreateSwapchain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(_window->window, &width, &height);
    while (width == 0 || height == 0) 
	{
        glfwGetFramebufferSize(_window->window, &width, &height);
        glfwWaitEvents();
    }

	_window->width = width;
	_window->height = height;

    vkDeviceWaitIdle(context->memoryManager->_device);
	
	destroyFullSwapchain();
	createFullSwapchain();
}

void VulkanEngine::createFullSwapchain()
{
	swapChainData = init_swapchain(_surface, context->memoryManager->_chosenGPU, _window, context->memoryManager->_device, context->memoryManager->_allocator); //

	std::cout << "renderpass" << std::endl;

	_renderPass = init_default_renderpass(swapChainData._colorImageFormat, swapChainData._depthImageFormat, VK_IMAGE_LAYOUT_GENERAL /*VK_IMAGE_LAYOUT_PRESENT_SRC_KHR*/); //

	std::cout << "framebuffer" << std::endl;
	init_framebuffers(); //

	init_commands(); //

	std::cout << "pipeline" << std::endl;
	init_pipelines(); //
}

void VulkanEngine::destroyFullSwapchain()
{
	// init_pipelines
	{
		for (auto& mat : resourceManager._materials)
		{
			mat.second->destroyPipeline(*context->memoryManager);
		}
	}

	// init_commands
	{
		for (int i = 0; i < FRAME_OVERLAP; i++) 
		{
			vkDestroyCommandPool(context->memoryManager->_device, _frames[i]._commandPool, nullptr);
		}
	}

	// init_framebuffers
	for (int i = 0; i < _framebuffers.size(); i++) 
	{
		vkDestroyFramebuffer(context->memoryManager->_device, _framebuffers[i], nullptr);
		vkDestroyImageView(context->memoryManager->_device, swapChainData._colorImageViews[i], nullptr);
	}

	// init_default_renderpass
	destroyRenderpass();

	// init_swapchain
	{
		vkDestroyImageView(context->memoryManager->_device, swapChainData._depthImageView, nullptr);
		vmaDestroyImage(context->memoryManager->_allocator, swapChainData._depthImage._image, swapChainData._depthImage._allocation);

		vkDestroySwapchainKHR(context->memoryManager->_device, swapChainData._swapchain, nullptr);
	}
}

void VulkanEngine::init(Window* window)
{
	_window = window;

	// Default Init
	init_vulkan();

	init_sync_structures();

	//everything went fine
	_isInitialized = true;

	std::cout << "end" << std::endl;
}

#include "Vk/DebugUtilsMessenger.hpp"

void VulkanCore::destroy()
{
	vk::DebugUtilsMessengerEXT::DestroyDebugUtilsMessengerEXT(instance, _debug_messenger, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void VulkanEngine::cleanup()
{	
	if (_isInitialized) 
	{
		//make sure the gpu has stopped doing its things
		vkDeviceWaitIdle(context->memoryManager->_device);

		{
			destroyFullSwapchain();

			// init_descriptors
			{

				// Destroy global and object descriptor buffers
				for (int i = 0; i < FRAME_OVERLAP; i++)
				{
					context->memoryManager->destroyBuffer(_frames[i].cameraBuffer);
					context->memoryManager->destroyBuffer(_frames[i].objectBuffer);
				}

				context->memoryManager->destroyBuffer(_sceneParameterBuffer);


				for (auto& mat : resourceManager._materials)
				{
					mat.second->destroyUniforms(*context->memoryManager);
				}
		
				// Destroy global and object descriptor layouts
				vkDestroyDescriptorSetLayout(context->memoryManager->_device, _objectSetLayout, nullptr);
				vkDestroyDescriptorSetLayout(context->memoryManager->_device, _globalSetLayout, nullptr);

				// Destroy descriptor pool
				vkDestroyDescriptorPool(context->memoryManager->_device, _descriptorPool, nullptr);
			}

			// init_sync_structures
			{
				vkDestroyFence(context->memoryManager->_device, context->memoryManager->_uploadContext._uploadFence, nullptr);

				for (int i = 0; i < FRAME_OVERLAP; i++) 
				{
					vkDestroySemaphore(context->memoryManager->_device, _frames[i]._presentSemaphore, nullptr);
					vkDestroySemaphore(context->memoryManager->_device, _frames[i]._renderSemaphore, nullptr);

					vkDestroyFence(context->memoryManager->_device, _frames[i]._renderFence, nullptr);
				}
			}
		}

		vkDestroySurfaceKHR(context->core.instance, _surface, nullptr);
	}
}

void VulkanEngine::recordColorRenderPass(VkCommandBuffer cmd, VkRenderPass renderPass, VkFramebuffer targetFramebuffer)
{
	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	// VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	// VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	//make a clear-color from frame number. This will flash with a 120 frame period.
	VkClearValue clearValue;
	float flash = abs(sin(_frameNumber / 120.f));
	clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };

	//clear depth at 1
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	//start the main renderpass. 
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(renderPass, {(uint32_t)_window->width, (uint32_t)_window->height}, targetFramebuffer);

	//connect clear values
	rpInfo.clearValueCount = 2;

	VkClearValue clearValues[] = { clearValue, depthClear };

	rpInfo.pClearValues = &clearValues[0];

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	draw_objects(cmd, _renderables.data(), (int) _renderables.size());	

	//finalize the render pass
	vkCmdEndRenderPass(cmd);

	// //finalize the command buffer (we can no longer add commands, but it can now be executed)
	// VK_CHECK(vkEndCommandBuffer(cmd));

	// //prepare the submission to the queue. 
	// //we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	// //we will signal the _renderSemaphore, to signal that rendering has finished

	// VkSubmitInfo submit = vkinit::submit_info(&cmd);
	// VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	// submit.pWaitDstStageMask = &waitStage;

	// submit.waitSemaphoreCount = 1;
	// submit.pWaitSemaphores = &get_current_frame()._presentSemaphore;

	// submit.signalSemaphoreCount = 1;
	// submit.pSignalSemaphores = &get_current_frame()._renderSemaphore;

	// //submit command buffer to the queue and execute it.
	// // _renderFence will now block until the graphic commands finish execution
	// VK_CHECK(vkQueueSubmit(context->memoryManager->_graphicsQueue, 1, &submit, get_current_frame()._renderFence));
}

// void VulkanEngineBase::recordCommand(VkCommandBuffer cmd)
void VulkanEngine::recordRenderPasses(VkFramebuffer targetFramebuffer)
{
	recordColorRenderPass(get_current_frame()._mainCommandBuffer, _renderPass, targetFramebuffer);

	//prepare the submission to the queue. 
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished

	VkSubmitInfo submit = vkinit::submit_info(&get_current_frame()._mainCommandBuffer);
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

void VulkanEngine::draw()
{
	//wait until the gpu has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(context->memoryManager->_device, 1, &get_current_frame()._renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(context->memoryManager->_device, 1, &get_current_frame()._renderFence));

	//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(get_current_frame()._mainCommandBuffer, 0));

	// framebuffer / image to use to render to viewport
	uint32_t swapchainImageIndex;

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

	// //naming it cmd for shorter writing
	// VkCommandBuffer cmd = get_current_frame()._mainCommandBuffer;

	// //begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	// VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	// VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	recordRenderPasses(_framebuffers[swapchainImageIndex]);

	// context->memoryManager->immediate_submit([&](VkCommandBuffer cmd) 
    // {
    //     context->memoryManager->changeImageLayout(cmd, swapChainData._colorImages[get_current_frame_id()], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    // });

	// //finalize the command buffer (we can no longer add commands, but it can now be executed)
	// VK_CHECK(vkEndCommandBuffer(cmd));

	// //prepare the submission to the queue. 
	// //we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	// //we will signal the _renderSemaphore, to signal that rendering has finished

	// VkSubmitInfo submit = vkinit::submit_info(&cmd);
	// VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	// submit.pWaitDstStageMask = &waitStage;

	// submit.waitSemaphoreCount = 1;
	// submit.pWaitSemaphores = &get_current_frame()._presentSemaphore;

	// submit.signalSemaphoreCount = 1;
	// submit.pSignalSemaphores = &get_current_frame()._renderSemaphore;

	// //submit command buffer to the queue and execute it.
	// // _renderFence will now block until the graphic commands finish execution
	// VK_CHECK(vkQueueSubmit(context->memoryManager->_graphicsQueue, 1, &submit, get_current_frame()._renderFence));

	//prepare present
	// this will put the image we just rendered to into the visible window.
	// we want to wait on the _renderSemaphore for that, 
	// as its necessary that drawing commands have finished before the image is displayed to the user
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

FrameData& VulkanEngine::get_current_frame()
{
	return _frames[_frameNumber % FRAME_OVERLAP];
}

int VulkanEngine::get_current_frame_id()
{
	return _frameNumber % FRAME_OVERLAP;
}


FrameData& VulkanEngine::get_last_frame()
{
	return _frames[(_frameNumber -1) % 2];
}

void VulkanCore::init()
{
	instance.createInstance();

	//grab the instance 
	_debug_messenger = vk::DebugUtilsMessengerEXT::createDebugMessenger(instance);// debugMessenger.debugMessenger;
}

void VulkanEngine::init_vulkan()
{
	if (glfwCreateWindowSurface(context->core.instance, _window->window, nullptr, &_surface) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create window surface!");
	}

	context->memoryManager->init(context->core.instance, _surface);
}
#include "SwapChain.hpp"
SwapChainData VulkanEngine::init_swapchain(VkSurfaceKHR _surface, PhysicalDevice _chosenGPU, Window* _window, VkDevice _device, VmaAllocator& _allocator)
{
	SwapChainData swapChainData;

	VkSurfaceFormatKHR surfaceFormat;

	// Create swapchain
	{
		SwapChainSupportDetails swapChainSupport = SwapChain::querySwapChainSupport(_surface, _chosenGPU);

		surfaceFormat = SwapChain::chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = SwapChain::chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = SwapChain::chooseSwapExtent(swapChainSupport.capabilities, _window);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = _surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT  /* to use with compute shader */ | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		QueueFamilyIndices indices = MemoryManager::findQueueFamilies(_chosenGPU, _surface);
		uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		if (indices.graphicsFamily != indices.presentFamily) 
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} 
		else 
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;
		
		if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &swapChainData._swapchain) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create swap chain!");
		}
	}


    // Get Color Images Views
	swapChainData._colorImageFormat = surfaceFormat.format;
	swapChainData.init(_device);



	// Create Swapchain's depth buffer
	{
		//depth image size will match the window
		VkExtent3D depthImageExtent = {
			(uint32_t) _window->width,
			(uint32_t) _window->height,
			1
		};

		//hardcoding the depth format to 32 bit float
		swapChainData._depthImageFormat = VK_FORMAT_D32_SFLOAT;

		//the depth image will be a image with the format we selected and Depth Attachment usage flag
		VkImageCreateInfo dimg_info = vkinit::image_create_info(swapChainData._depthImageFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

		//for the depth image, we want to allocate it from gpu local memory
		VmaAllocationCreateInfo dimg_allocinfo = {};
		dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		//allocate and create the image
		vmaCreateImage(_allocator, &dimg_info, &dimg_allocinfo, &swapChainData._depthImage._image, &swapChainData._depthImage._allocation, nullptr);

		//build a image-view for the depth image to use for rendering
		VkImageViewCreateInfo dview_info = vkinit::imageview_create_info(swapChainData._depthImageFormat, swapChainData._depthImage._image, VK_IMAGE_ASPECT_DEPTH_BIT);;

		VK_CHECK(vkCreateImageView(_device, &dview_info, nullptr, &swapChainData._depthImageView));
	}

	return swapChainData;
}

void SwapChainData::init(VkDevice device)
{
	uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, _swapchain, &imageCount, nullptr);
    _colorImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, _swapchain, &imageCount, _colorImages.data());

	_colorImageViews.resize(imageCount);

	// Create Views
	for (int i = 0; i < _colorImageViews.size(); i++)
	{
		VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = _colorImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = _colorImageFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &viewInfo, nullptr, &_colorImageViews[i]) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create texture image view!");
        }
	}
}

VkRenderPass VulkanEngineBase::init_default_renderpass(VkFormat _colorImageFormat, VkFormat _depthImageFormat, VkImageLayout finalLayout)
{
	return context->memoryManager->init_default_renderpass(_colorImageFormat, _depthImageFormat, finalLayout);
}

void VulkanEngineBase::destroyRenderpass()
{
	vkDestroyRenderPass(context->memoryManager->_device, _renderPass, nullptr);
}

void VulkanEngine::init_framebuffers()
{
	std::cout << "Frame buffer resize " << (uint32_t)_window->width << " / " << (uint32_t)_window->height << std::endl;
	//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	VkFramebufferCreateInfo fb_info = vkinit::framebuffer_create_info(_renderPass, {(uint32_t)_window->width, (uint32_t)_window->height});

	const int swapchain_imagecount = (int) swapChainData._colorImages.size();
	_framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

	for (int i = 0; i < swapchain_imagecount; i++) 
	{

		VkImageView attachments[2];
		attachments[0] = swapChainData._colorImageViews[i];
		attachments[1] = swapChainData._depthImageView;

		fb_info.pAttachments = attachments;
		fb_info.attachmentCount = 2;
		VK_CHECK(vkCreateFramebuffer(context->memoryManager->_device, &fb_info, nullptr, &_framebuffers[i]));
	}
}

void VulkanEngine::init_commandPool()
{
	//create a command pool for commands submitted to the graphics queue.
	//we also want the pool to allow for resetting of individual command buffers
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(context->memoryManager->queueFamilies.graphicsFamily.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	
	for (int i = 0; i < FRAME_OVERLAP; i++) 
	{
		VK_CHECK(vkCreateCommandPool(context->memoryManager->_device, &commandPoolInfo, nullptr, &_frames[i]._commandPool));
	}
}

void VulkanEngine::init_commands()
{
	init_commandPool();

	for (int i = 0; i < FRAME_OVERLAP; i++) 
	{
		//allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_frames[i]._commandPool, 1);

		VK_CHECK(vkAllocateCommandBuffers(context->memoryManager->_device, &cmdAllocInfo, &_frames[i]._mainCommandBuffer));
	}


	// VkCommandPoolCreateInfo uploadCommandPoolInfo = vkinit::command_pool_create_info(context->memoryManager->_graphicsQueueFamily);
	// //create pool for upload context
	// VK_CHECK(vkCreateCommandPool(context->memoryManager->_device, &uploadCommandPoolInfo, nullptr, &context->memoryManager->_uploadContext._commandPool));

}

void VulkanEngine::init_sync_structures()
{
	//create syncronization structures
	//one fence to control when the gpu has finished rendering the frame,
	//and 2 semaphores to syncronize rendering with swapchain
	//we want the fence to start signalled so we can wait on it on the first frame
	VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);

	VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

	for (int i = 0; i < FRAME_OVERLAP; i++) 
	{

		VK_CHECK(vkCreateFence(context->memoryManager->_device, &fenceCreateInfo, nullptr, &_frames[i]._renderFence));

		VK_CHECK(vkCreateSemaphore(context->memoryManager->_device, &semaphoreCreateInfo, nullptr, &_frames[i]._presentSemaphore));
		VK_CHECK(vkCreateSemaphore(context->memoryManager->_device, &semaphoreCreateInfo, nullptr, &_frames[i]._renderSemaphore));

	}
	
	VkFenceCreateInfo uploadFenceCreateInfo = vkinit::fence_create_info();

	VK_CHECK(vkCreateFence(context->memoryManager->_device, &uploadFenceCreateInfo, nullptr, &context->memoryManager->_uploadContext._uploadFence));
}

void VulkanEngine::addShaderModule(const std::string& path)
{
	VkShaderModule meshVertShader;
	if (VulkanContext::load_shader_module(context->memoryManager->_device, path.c_str(), &meshVertShader))
	{
		resourceManager.shaderModules.emplace(path, meshVertShader);
	}
	else 
	{
		std::cout << "Error when building the mesh vertex shader module" << std::endl;
	}
}

void VulkanEngine::init_pipelines()
{
	// std::unordered_map<std::string, VkShaderModule> shaderModules = loadShaders(_device);
	
	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
	PipelineBuilder pipelineBuilder = GraphicsMaterial::getPipelineBuilder(*_window);

	for (auto& matPair : resourceManager._materials)
	{
		assert(matPair.second.get() != nullptr);

		GraphicsMaterial& mat = *matPair.second;
		mat.makePipelineLayout(context->memoryManager->_device, _globalSetLayout, _objectSetLayout);
		mat.makePipeline(context->memoryManager->_device, _renderPass, pipelineBuilder, resourceManager.shaderModules);
	}
}


static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}
#include "Vk/ShaderModule.hpp"
bool VulkanContext::load_shader_module(VkDevice _device, const char* filePath, VkShaderModule* outShaderModule)
{
	auto shaderCode = readFile(filePath);
	vk::ShaderModule fShaderModule (_device, shaderCode);
	
	*outShaderModule = fShaderModule.shaderModule;
	return true;
}

void VulkanEngine::draw_objects(VkCommandBuffer cmd, RenderObject* first, int count)
{
	//make a model view matrix for rendering the object
	//camera view
	glm::vec3 camPos = { 0.f,-6.f,-10.f };

	glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
	//camera projection
	glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
	projection[1][1] *= -1;	

	GPUCameraData camData;
	camData.proj = projection;
	camData.view = view;
	camData.viewproj = projection * view;

	void* data;
	vmaMapMemory(context->memoryManager->_allocator, get_current_frame().cameraBuffer._allocation, &data);

	memcpy(data, &camData, sizeof(GPUCameraData));

	vmaUnmapMemory(context->memoryManager->_allocator, get_current_frame().cameraBuffer._allocation);

	for (auto& matPair : resourceManager._materials)
	{
		assert(matPair.second.get() != nullptr);
		matPair.second->updateUniforms(context->memoryManager->_allocator, get_current_frame_id());
	}

	float framed = (_frameNumber / 120.f);

	_sceneParameters.ambientColor = { sin(framed),0,cos(framed),1 };

	char* sceneData;
	vmaMapMemory(context->memoryManager->_allocator, _sceneParameterBuffer._allocation , (void**)&sceneData);

	int frameIndex = _frameNumber % FRAME_OVERLAP;

	sceneData += context->memoryManager->pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameIndex;

	memcpy(sceneData, &_sceneParameters, sizeof(GPUSceneData));

	vmaUnmapMemory(context->memoryManager->_allocator, _sceneParameterBuffer._allocation);


	void* objectData;
	vmaMapMemory(context->memoryManager->_allocator, get_current_frame().objectBuffer._allocation, &objectData);
	
	GPUObjectData* objectSSBO = (GPUObjectData*)objectData;
	
	for (int i = 0; i < count; i++)
	{
		RenderObject& object = first[i];
		objectSSBO[i].modelMatrix = object.transformMatrix;
	}
	
	vmaUnmapMemory(context->memoryManager->_allocator, get_current_frame().objectBuffer._allocation);

	GPUMesh* lastMesh = nullptr;
	GraphicsMaterial* lastMaterial = nullptr;
	
	for (int i = 0; i < count; i++)
	{
		RenderObject& object = first[i];

		//only bind the pipeline if it doesnt match with the already bound one
		if (object.material != lastMaterial) 
		{
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);

			object.material->bindUniforms(cmd, get_current_frame().globalDescriptor, get_current_frame().objectDescriptor, frameIndex, context->memoryManager->_gpuProperties);

			lastMaterial = object.material;
		}

		glm::mat4 model = object.transformMatrix;
		//final render matrix, that we are calculating on the cpu
		glm::mat4 mesh_matrix = model;

		MeshPushConstants constants;
		constants.render_matrix = mesh_matrix;

		//upload the mesh to the gpu via pushconstants
		vkCmdPushConstants(cmd, object.material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

		//only bind the mesh if its a different one from last bind
		if (object.mesh != lastMesh) {
			//bind the mesh vertex buffer with offset 0
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->_vertexBuffer._buffer, &offset);
			lastMesh = object.mesh;
		}
		//we can now draw
		vkCmdDraw(cmd, (uint32_t) object.mesh->size,  1,  0, i);
	}
}

void VulkanEngine::init_descriptors()
{
	//create a descriptor pool that will hold 10 uniform buffers
	{
		std::vector<VkDescriptorPoolSize> sizes =
		{
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = 0;
		pool_info.maxSets = 10;
		pool_info.poolSizeCount = (uint32_t)sizes.size();
		pool_info.pPoolSizes = sizes.data();
		
		vkCreateDescriptorPool(context->memoryManager->_device, &pool_info, nullptr, &_descriptorPool);	
	}
	
	// Create global descriptor set layout
	{
		VkDescriptorSetLayoutBinding cameraBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);
		VkDescriptorSetLayoutBinding sceneBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
		
		VkDescriptorSetLayoutBinding bindings[] = { cameraBind,sceneBind };

		VkDescriptorSetLayoutCreateInfo setinfo = {};
		setinfo.bindingCount = 2;
		setinfo.flags = 0;
		setinfo.pNext = nullptr;
		setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setinfo.pBindings = bindings;

		vkCreateDescriptorSetLayout(context->memoryManager->_device, &setinfo, nullptr, &_globalSetLayout);
	}

	// Create object description set layout
	{
		VkDescriptorSetLayoutBinding objectBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

		VkDescriptorSetLayoutCreateInfo set2info = {};
		set2info.bindingCount = 1;
		set2info.flags = 0;
		set2info.pNext = nullptr;
		set2info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		set2info.pBindings = &objectBind;

		vkCreateDescriptorSetLayout(context->memoryManager->_device, &set2info, nullptr, &_objectSetLayout);
	}

	for (auto& matPair : resourceManager._materials)
	{
		matPair.second->createUniforms(*this, context->memoryManager->_device, _descriptorPool);
	}

	const size_t sceneParamBufferSize = FRAME_OVERLAP * context->memoryManager->pad_uniform_buffer_size(sizeof(GPUSceneData));

	_sceneParameterBuffer = context->memoryManager->create_buffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	
	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		_frames[i].cameraBuffer = context->memoryManager->create_buffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		const int MAX_OBJECTS = 10000;
		_frames[i].objectBuffer = context->memoryManager->create_buffer(sizeof(GPUObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		// Allocate global descriptor set
		{
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.pNext = nullptr;
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = _descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &_globalSetLayout;

			vkAllocateDescriptorSets(context->memoryManager->_device, &allocInfo, &_frames[i].globalDescriptor);
		}

		// Allocate object descriptor set
		{
			VkDescriptorSetAllocateInfo objectSetAlloc = {};
			objectSetAlloc.pNext = nullptr;
			objectSetAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			objectSetAlloc.descriptorPool = _descriptorPool;
			objectSetAlloc.descriptorSetCount = 1;
			objectSetAlloc.pSetLayouts = &_objectSetLayout;

			vkAllocateDescriptorSets(context->memoryManager->_device, &objectSetAlloc, &_frames[i].objectDescriptor);
		}

		VkDescriptorBufferInfo cameraInfo;
		cameraInfo.buffer = _frames[i].cameraBuffer._buffer;
		cameraInfo.offset = 0;
		cameraInfo.range = sizeof(GPUCameraData);

		VkDescriptorBufferInfo sceneInfo;
		sceneInfo.buffer = _sceneParameterBuffer._buffer;
		sceneInfo.offset = 0;
		sceneInfo.range = sizeof(GPUSceneData);

		VkDescriptorBufferInfo objectBufferInfo;
		objectBufferInfo.buffer = _frames[i].objectBuffer._buffer;
		objectBufferInfo.offset = 0;
		objectBufferInfo.range = sizeof(GPUObjectData) * MAX_OBJECTS;


		VkWriteDescriptorSet cameraWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _frames[i].globalDescriptor,&cameraInfo,0);
		
		VkWriteDescriptorSet sceneWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, _frames[i].globalDescriptor, &sceneInfo, 1);

		VkWriteDescriptorSet objectWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, _frames[i].objectDescriptor, &objectBufferInfo, 0);

		VkWriteDescriptorSet setWrites[] = { cameraWrite,sceneWrite,objectWrite };

		vkUpdateDescriptorSets(context->memoryManager->_device, 3, setWrites, 0, nullptr);
	}
}
