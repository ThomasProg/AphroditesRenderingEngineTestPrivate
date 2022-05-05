
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

	std::cout << "window : " << width << " / "  << height << std::endl;

	_window->width = width;
	_window->height = height;

    vkDeviceWaitIdle(context->memoryManager->_device);
	
	destroyFullSwapchain();
	createFullSwapchain();
}

void VulkanEngine::createFullSwapchain()
{

}

void VulkanEngine::destroyFullSwapchain()
{
	// // init_pipelines
	// {
	// 	for (auto& mat : resourceManager._materials)
	// 	{
	// 		mat.second->destroyPipeline(*context->memoryManager);
	// 	}
	// }

	// init_commands
	{
		for (int i = 0; i < FRAME_OVERLAP; i++) 
		{
			vkDestroyCommandPool(context->memoryManager->_device, _frames[i]._commandPool, nullptr);
		}
	}

	// init_swapchain
	{
		vkDestroySwapchainKHR(context->memoryManager->_device, swapChainData._swapchain, nullptr);
	}
}

void VulkanEngine::init(Window* window)
{
	_window = window;

	// Default Init
	init_vulkan();

	init_sync_structures();

	firstSetup(*context->memoryManager);

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

			// // init_descriptors
			// {

			// 	// Destroy global and object descriptor buffers
			// 	for (int i = 0; i < FRAME_OVERLAP; i++)
			// 	{
			// 		context->memoryManager->destroyBuffer(_frames[i].cameraBuffer);
			// 		context->memoryManager->destroyBuffer(_frames[i].objectBuffer);
			// 	}

			// 	context->memoryManager->destroyBuffer(_sceneParameterBuffer);


			// 	for (auto& mat : resourceManager._materials)
			// 	{
			// 		mat.second->destroyUniforms(*context->memoryManager);
			// 	}
		
			// 	// Destroy global and object descriptor layouts
			// 	vkDestroyDescriptorSetLayout(context->memoryManager->_device, _objectSetLayout, nullptr);
			// 	vkDestroyDescriptorSetLayout(context->memoryManager->_device, _globalSetLayout, nullptr);

			// 	// Destroy descriptor pool
			// 	vkDestroyDescriptorPool(context->memoryManager->_device, _descriptorPool, nullptr);
			// }
			lastDestroy(*context->memoryManager);

			// init_sync_structures
			{
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

// void VulkanEngine::recordColorRenderPass(VkCommandBuffer cmd, VkRenderPass renderPass, VkFramebuffer targetFramebuffer)
// {
// 	// //make a clear-color from frame number. This will flash with a 120 frame period.
// 	// VkClearValue clearValue;
// 	// float flash = abs(sin(_frameNumber / 120.f));
// 	// clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };

// 	// //clear depth at 1
// 	// VkClearValue depthClear;
// 	// depthClear.depthStencil.depth = 1.f;

// 	// //start the main renderpass. 
// 	// //We will use the clear color from above, and the framebuffer of the index the swapchain gave us
// 	// VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(renderPass, {(uint32_t)_window->width, (uint32_t)_window->height}, targetFramebuffer);

// 	// //connect clear values
// 	// rpInfo.clearValueCount = 2;

// 	// VkClearValue clearValues[] = { clearValue, depthClear };

// 	// rpInfo.pClearValues = &clearValues[0];

// 	// vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

// 	// draw_objects(cmd, _renderables.data(), (int) _renderables.size());	

// 	// //finalize the render pass
// 	// vkCmdEndRenderPass(cmd);
// }

// void VulkanEngineBase::recordCommand(VkCommandBuffer cmd)
// void VulkanEngine::recordRenderPasses()
// {

// }

void VulkanEngine::draw()
{

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
	swapChainData._depthImageFormat = VK_FORMAT_D32_SFLOAT;
	swapChainData.init(_device);

	return swapChainData;
}

void SwapChainData::init(VkDevice device)
{
	uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, _swapchain, &imageCount, nullptr);
    _colorImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, _swapchain, &imageCount, _colorImages.data());
}

VkRenderPass VulkanEngineBase::init_default_renderpass(VkFormat _colorImageFormat, VkFormat _depthImageFormat, VkImageLayout finalLayout)
{
	return context->memoryManager->init_default_renderpass(_colorImageFormat, _depthImageFormat, finalLayout);
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
