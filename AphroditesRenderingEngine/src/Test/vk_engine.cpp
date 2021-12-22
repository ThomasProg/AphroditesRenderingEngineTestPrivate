
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

//we want to immediately abort when there is an error. In normal engines this would give an error message to the user, or perform a dump of state.
using namespace std;
#define VK_CHECK(x) x                                                 
	//do                                                              \
	//{                                                               \
	//	VkResult err = x;                                           \
	//	if (err)                                                    \
	//	{                                                           \
	//		std::cout <<"Detected Vulkan error: " << err << std::endl; \
	//		abort();                                                \
	//	}                                                           \
	//} while (0)


void Material::createLayout(VkDevice device)
{
	VkDescriptorSetLayoutBinding textureBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	VkDescriptorSetLayoutBinding testBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);

	constexpr size_t nbBindings = 2;
	VkDescriptorSetLayoutBinding  t[nbBindings] = {textureBind, testBind};

	VkDescriptorSetLayoutCreateInfo set3info = {};
	set3info.bindingCount = nbBindings;
	set3info.flags = 0;
	set3info.pNext = nullptr;
	set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set3info.pBindings = t;

	vkCreateDescriptorSetLayout(device, &set3info, nullptr, &shaderLayout);
}

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

    vkDeviceWaitIdle(_device);
	
	destroyFullSwapchain();
	createFullSwapchain();
}

void VulkanEngine::createFullSwapchain()
{
	swapChainData = init_swapchain(_surface, _chosenGPU, _window, _device, _allocator); //

	std::cout << "renderpass" << std::endl;

	init_default_renderpass(); //

	std::cout << "framebuffer" << std::endl;
	init_framebuffers(); //

	init_commands(); //

	std::cout << "pipeline" << std::endl;
	init_pipelines(); //
}

void Material::destroyPipeline(VkDevice _device)
{
	vkDestroyPipeline(_device, pipeline, nullptr);
	vkDestroyPipelineLayout(_device, pipelineLayout, nullptr);
}

void VulkanEngine::destroyFullSwapchain()
{
	// init_pipelines
	{
		for (auto mat : resourceManager._materials)
		{
			mat.second.destroyPipeline(_device);
		}
	}

	// init_commands
	{
		vkDestroyCommandPool(_device, _uploadContext._commandPool, nullptr);

		for (int i = 0; i < FRAME_OVERLAP; i++) 
		{
			vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);
		}
	}

	// init_framebuffers
	for (int i = 0; i < _framebuffers.size(); i++) 
	{
		vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
		vkDestroyImageView(_device, swapChainData._colorImageViews[i], nullptr);
	}

	// init_default_renderpass
	vkDestroyRenderPass(_device, _renderPass, nullptr);

	// init_swapchain
	{
		vkDestroyImageView(_device, swapChainData._depthImageView, nullptr);
		vmaDestroyImage(_allocator, swapChainData._depthImage._image, swapChainData._depthImage._allocation);

		vkDestroySwapchainKHR(_device, swapChainData._swapchain, nullptr);
	}
}

void VulkanEngine::init(Window* window)
{
	_window = window;

	// Default Init
	init_vulkan();


	init_sync_structures();

	init_descriptors();

	createFullSwapchain();
	// std::cout << "Swapchain" << std::endl;

	// // SwapChain Recreation
	// {
	// 	swapChainData = init_swapchain(_surface, _chosenGPU, _window, _device, _allocator); //

	// 	std::cout << "renderpass" << std::endl;

	// 	init_default_renderpass(); //

	// 	std::cout << "framebuffer" << std::endl;
	// 	init_framebuffers(); //

	// 	init_commands(); //

	// 	std::cout << "pipeline" << std::endl;
	// 	init_pipelines(); //
	// }

	// // Load Resources
	// std::cout << "images" << std::endl;	
	// load_images();

	// std::cout << "mesh" << std::endl;
	// load_meshes();

	// // Load Scene
	// init_scene();


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

void ResourceManager::clear(VkDevice _device, VmaAllocator _allocator)
{
	// Mesh buffer
	_meshes["monkey"].destroy(_allocator);
	_meshes["triangle"].destroy(_allocator);
	_meshes["empire"].destroy(_allocator);

	// lost empire image
	GPUTexture lostEmpire = _loadedTextures["empire_diffuse"];
	vmaDestroyImage(_allocator, lostEmpire.image._image, lostEmpire.image._allocation);
	vkDestroyImageView(_device, lostEmpire.imageView, nullptr);	
}

void VulkanEngine::cleanup()
{	
	if (_isInitialized) {
		
		//make sure the gpu has stopped doing its things
		vkDeviceWaitIdle(_device);

		{
			// _mainDeletionQueue.flush();

			vkDestroySampler(_device, blockySampler, nullptr);

			// // load_images / load_meshes
			// {
			// 	// Mesh buffer
			// 	resourceManager._meshes["monkey"].destroy(_allocator);
			// 	resourceManager._meshes["triangle"].destroy(_allocator);
			// 	resourceManager._meshes["empire"].destroy(_allocator);

			// 	// lost empire image
			// 	Texture lostEmpire = _loadedTextures["empire_diffuse"];
			// 	vmaDestroyImage(_allocator, lostEmpire.image._image, lostEmpire.image._allocation);
			// 	vkDestroyImageView(_device, lostEmpire.imageView, nullptr);	
			// }
			resourceManager.clear(_device, _allocator);


			destroyFullSwapchain();
			// // init_pipelines
			// {
			// 	vkDestroyPipeline(_device, meshPipeline, nullptr);
			// 	vkDestroyPipeline(_device, texPipeline, nullptr);

			// 	vkDestroyPipelineLayout(_device, meshPipLayout, nullptr);
			// 	vkDestroyPipelineLayout(_device, texturedPipeLayout, nullptr);
			// }

			// // init_commands
			// {
			// 	vkDestroyCommandPool(_device, _uploadContext._commandPool, nullptr);

			// 	for (int i = 0; i < FRAME_OVERLAP; i++) 
			// 	{
			// 		vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);
			// 	}
			// }

			// // init_framebuffers
			// for (int i = 0; i < _framebuffers.size(); i++) 
			// {
			// 	vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
			// 	vkDestroyImageView(_device, swapChainData._colorImageViews[i], nullptr);
			// }

			// // init_default_renderpass
			// vkDestroyRenderPass(_device, _renderPass, nullptr);

			// // init_swapchain
			// {
			// 	vkDestroyImageView(_device, swapChainData._depthImageView, nullptr);
			// 	vmaDestroyImage(_allocator, swapChainData._depthImage._image, swapChainData._depthImage._allocation);

			// 	vkDestroySwapchainKHR(_device, swapChainData._swapchain, nullptr);
			// }

			// init_descriptors
			{
				vmaDestroyBuffer(_allocator, _sceneParameterBuffer._buffer, _sceneParameterBuffer._allocation);

				vkDestroyDescriptorSetLayout(_device, _objectSetLayout, nullptr);
				vkDestroyDescriptorSetLayout(_device, _globalSetLayout, nullptr);
				// vkDestroyDescriptorSetLayout(_device, _singleTextureSetLayout, nullptr);
				// texturedShaderUniforms.destroy(_device);

				vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);

				for (int i = 0; i < FRAME_OVERLAP; i++)
				{
					vmaDestroyBuffer(_allocator, _frames[i].cameraBuffer._buffer, _frames[i].cameraBuffer._allocation);

					vmaDestroyBuffer(_allocator, _frames[i].objectBuffer._buffer, _frames[i].objectBuffer._allocation);

					// // Test Material Destroy
					// vmaDestroyBuffer(_allocator, _frames[i].testBuffer._buffer, _frames[i].testBuffer._allocation);
				}

				for (auto mat : resourceManager._materials)
				{
					//for (int i = 0; i < FRAME_OVERLAP; i++)
					for  (AllocatedBuffer& buffer : mat.second.uniformBuffers)
					{
						// Test Material Destroy
						// vmaDestroyBuffer(_allocator, _frames[i].testBuffer._buffer, _frames[i].testBuffer._allocation);
						vmaDestroyBuffer(_allocator, buffer._buffer, buffer._allocation);
					}

					mat.second.destroyLayout(_device);
				}
			}

			// init_sync_structures
			{
				vkDestroyFence(_device, _uploadContext._uploadFence, nullptr);

				for (int i = 0; i < FRAME_OVERLAP; i++) 
				{
					vkDestroySemaphore(_device, _frames[i]._presentSemaphore, nullptr);
					vkDestroySemaphore(_device, _frames[i]._renderSemaphore, nullptr);

					vkDestroyFence(_device, _frames[i]._renderFence, nullptr);
				}
			}

			// init_vulkan
			vmaDestroyAllocator(_allocator);
		}

		vkDestroySurfaceKHR(core.instance, _surface, nullptr);
		vkDestroyDevice(_device, nullptr);
		core.destroy();
	}
}

void VulkanEngine::draw()
{
	
	//wait until the gpu has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(_device, 1, &get_current_frame()._renderFence));

	//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(get_current_frame()._mainCommandBuffer, 0));

	//request image from the swapchain
	uint32_t swapchainImageIndex;
	VkResult result = vkAcquireNextImageKHR(_device, swapChainData._swapchain, 1000000000, get_current_frame()._presentSemaphore, nullptr, &swapchainImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) 
	{
		recreateSwapchain();
		return;
	} 
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	//naming it cmd for shorter writing
	VkCommandBuffer cmd = get_current_frame()._mainCommandBuffer;

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	//make a clear-color from frame number. This will flash with a 120 frame period.
	VkClearValue clearValue;
	float flash = abs(sin(_frameNumber / 120.f));
	clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };

	//clear depth at 1
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	//start the main renderpass. 
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(_renderPass, {(uint32_t)_window->width, (uint32_t)_window->height}, _framebuffers[swapchainImageIndex]);

	//connect clear values
	rpInfo.clearValueCount = 2;

	VkClearValue clearValues[] = { clearValue, depthClear };

	rpInfo.pClearValues = &clearValues[0];
	
	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	draw_objects(cmd, _renderables.data(), _renderables.size());	

	//finalize the render pass
	vkCmdEndRenderPass(cmd);
	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(cmd));

	//prepare the submission to the queue. 
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished

	VkSubmitInfo submit = vkinit::submit_info(&cmd);
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &get_current_frame()._presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &get_current_frame()._renderSemaphore;

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, get_current_frame()._renderFence));

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

	result = vkQueuePresentKHR(_graphicsQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		recreateSwapchain();
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	// // Added
	// {
	// 	VkResult result = vkAcquireNextImageKHR(_device, swapChainData._swapchain, UINT64_MAX, get_current_frame()._renderSemaphore, VK_NULL_HANDLE, &swapchainImageIndex);

	// 	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
	// 		vkDeviceWaitIdle(_device);
	// 		destroyFullSwapchain();
	// 		createFullSwapchain();
	// 		return;
	// 	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
	// 		throw std::runtime_error("failed to acquire swap chain image!");
	// 	}
	// }

	//increase the number of frames drawn
	_frameNumber++;
}

void VulkanEngine::run()
{
	draw();
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

#include "LogicalDevice.hpp"
#include "Presenter.hpp"

void VulkanCore::init()
{
	instance.createInstance();

	//grab the instance 
	_debug_messenger = vk::DebugUtilsMessengerEXT::createDebugMessenger(instance);// debugMessenger.debugMessenger;
}

void VulkanEngine::init_vulkan()
{
	// _window->onResize = [&](Window* window)
	// {
	// 	vkDeviceWaitIdle(_device);
	// 	destroyFullSwapchain();
	// 	createFullSwapchain();
	// };

	core.init();

	if (glfwCreateWindowSurface(core.instance, _window->window, nullptr, &_surface) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create window surface!");
	}

	LogicalDevice device;

	_chosenGPU.physicalDevice = Presenter::pickPhysicalDevice(core.instance, _surface);
	QueueFamilyIndices indices = Presenter::findQueueFamilies(_chosenGPU, _surface);
	device.init(indices, _chosenGPU);


	_device = device;

	_graphicsQueue = device.graphicsQueue;

	_graphicsQueueFamily =  indices.graphicsFamily.value();

	//initialize the memory allocator
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = _chosenGPU;
	allocatorInfo.device = _device;
	allocatorInfo.instance = core.instance;
	vmaCreateAllocator(&allocatorInfo, &_allocator);

	// _mainDeletionQueue.push_function([&]() {
	// 	vmaDestroyAllocator(_allocator);
	// });

	vkGetPhysicalDeviceProperties(_chosenGPU, &_gpuProperties);

	std::cout << "The gpu has a minimum buffer alignement of " << _gpuProperties.limits.minUniformBufferOffsetAlignment << std::endl;
}

SwapChainData VulkanEngine::init_swapchain(VkSurfaceKHR _surface, PhysicalDevice _chosenGPU, Window* _window, VkDevice _device, VmaAllocator& _allocator)
{
	SwapChainData swapChainData;

	// Create swapchain
    SwapChainSupportDetails swapChainSupport = SwapChain::querySwapChainSupport(_surface, _chosenGPU);

	std::cout << "SwapChainSupport SIZE : "  <<  swapChainSupport.formats.size() << std::endl;

    VkSurfaceFormatKHR surfaceFormat = SwapChain::chooseSwapSurfaceFormat(swapChainSupport.formats);
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
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	vk::Surface surface;
	surface.surface = _surface;
	QueueFamilyIndices indices = Presenter::findQueueFamilies(_chosenGPU, surface.surface);
	uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
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

    // Get Images
    vkGetSwapchainImagesKHR(_device, swapChainData._swapchain, &imageCount, nullptr);
    swapChainData._colorImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_device, swapChainData._swapchain, &imageCount, swapChainData._colorImages.data());

	swapChainData._colorImageViews.resize(swapChainData._colorImages.size());

	for (uint32_t i = 0; i < swapChainData._colorImageViews.size(); i++) 
	{
		VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapChainData._colorImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = surfaceFormat.format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(_device, &viewInfo, nullptr, &swapChainData._colorImageViews[i]) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create texture image view!");
        }
	}

	swapChainData._colorImageFormat = surfaceFormat.format;

	// _mainDeletionQueue.push_function([=]() {
	// 	vkDestroySwapchainKHR(_device, swapChainData._swapchain, nullptr);
	// });

	//depth image size will match the window
	VkExtent3D depthImageExtent = {
		_window->width,
		_window->height,
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

	// //add to deletion queues
	// _mainDeletionQueue.push_function([=]() {
	// 	vkDestroyImageView(_device, swapChainData._depthImageView, nullptr);
	// 	vmaDestroyImage(_allocator, swapChainData._depthImage._image, swapChainData._depthImage._allocation);
	// });


	return swapChainData;
}

void VulkanEngine::init_default_renderpass()
{
	//we define an attachment description for our main color image
	//the attachment is loaded as "clear" when renderpass start
	//the attachment is stored when renderpass ends
	//the attachment layout starts as "undefined", and transitions to "Present" so its possible to display it
	//we dont care about stencil, and dont use multisampling

	VkAttachmentDescription color_attachment = {};
	color_attachment.format = swapChainData._colorImageFormat;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depth_attachment = {};
	// Depth attachment
	depth_attachment.flags = 0;
	depth_attachment.format = swapChainData._depthImageFormat;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = {};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	//hook the depth attachment into the subpass
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	//1 dependency, which is from "outside" into the subpass. And we can read or write color
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


	//array of 2 attachments, one for the color, and other for depth
	VkAttachmentDescription attachments[2] = { color_attachment,depth_attachment };

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	//2 attachments from said array
	render_pass_info.attachmentCount = 2;
	render_pass_info.pAttachments = &attachments[0];
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;
	
	VK_CHECK(vkCreateRenderPass(_device, &render_pass_info, nullptr, &_renderPass));

	// _mainDeletionQueue.push_function([=]() {
	// 	vkDestroyRenderPass(_device, _renderPass, nullptr);
	// });
}

void VulkanEngine::init_framebuffers()
{
	std::cout << "Frame buffer resize " << (uint32_t)_window->width << " / " << (uint32_t)_window->height << std::endl;
	//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	VkFramebufferCreateInfo fb_info = vkinit::framebuffer_create_info(_renderPass, {(uint32_t)_window->width, (uint32_t)_window->height});

	const uint32_t swapchain_imagecount = swapChainData._colorImages.size();
	_framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

	for (int i = 0; i < swapchain_imagecount; i++) {

		VkImageView attachments[2];
		attachments[0] = swapChainData._colorImageViews[i];
		attachments[1] = swapChainData._depthImageView;

		fb_info.pAttachments = attachments;
		fb_info.attachmentCount = 2;
		VK_CHECK(vkCreateFramebuffer(_device, &fb_info, nullptr, &_framebuffers[i]));

		// _mainDeletionQueue.push_function([=]() {
		// 	vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
		// 	vkDestroyImageView(_device, swapChainData._colorImageViews[i], nullptr);
		// });
	}
}

void VulkanEngine::init_commands()
{
	//create a command pool for commands submitted to the graphics queue.
	//we also want the pool to allow for resetting of individual command buffers
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	
	for (int i = 0; i < FRAME_OVERLAP; i++) 
	{

	
		VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_frames[i]._commandPool));

		//allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_frames[i]._commandPool, 1);

		VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_frames[i]._mainCommandBuffer));

		// _mainDeletionQueue.push_function([=]() {
		// 	vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);
		// });
	}


	VkCommandPoolCreateInfo uploadCommandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily);
	//create pool for upload context
	VK_CHECK(vkCreateCommandPool(_device, &uploadCommandPoolInfo, nullptr, &_uploadContext._commandPool));

	// _mainDeletionQueue.push_function([=]() {
	// 	vkDestroyCommandPool(_device, _uploadContext._commandPool, nullptr);
	// });
}

void VulkanEngine::init_sync_structures()
{
	//create syncronization structures
	//one fence to control when the gpu has finished rendering the frame,
	//and 2 semaphores to syncronize rendering with swapchain
	//we want the fence to start signalled so we can wait on it on the first frame
	VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);

	VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

	for (int i = 0; i < FRAME_OVERLAP; i++) {

		VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_frames[i]._renderFence));

	// //enqueue the destruction of the fence
	// _mainDeletionQueue.push_function([=]() {
	// 	vkDestroyFence(_device, _frames[i]._renderFence, nullptr);
	// 	});


	VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._presentSemaphore));
	VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._renderSemaphore));

	// //enqueue the destruction of semaphores
	// _mainDeletionQueue.push_function([=]() {
	// 	vkDestroySemaphore(_device, _frames[i]._presentSemaphore, nullptr);
	// 	vkDestroySemaphore(_device, _frames[i]._renderSemaphore, nullptr);
	// 	});
	}

	
	VkFenceCreateInfo uploadFenceCreateInfo = vkinit::fence_create_info();

	VK_CHECK(vkCreateFence(_device, &uploadFenceCreateInfo, nullptr, &_uploadContext._uploadFence));
	// _mainDeletionQueue.push_function([=]() {
	// 	vkDestroyFence(_device, _uploadContext._uploadFence, nullptr);
	// });
}

PipelineBuilder Material::getPipelineBuilder(Window& window)
{
	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
	PipelineBuilder pipelineBuilder;

	//vertex input controls how to read vertices from vertex buffers. We arent using it yet
	pipelineBuilder._vertexInputInfo = vkinit::vertex_input_state_create_info();

	//input assembly is the configuration for drawing triangle lists, strips, or individual points.
	//we are just going to draw triangle list
	pipelineBuilder._inputAssembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	//build viewport and scissor from the swapchain extents
	pipelineBuilder._viewport.x = 0.0f;
	pipelineBuilder._viewport.y = 0.0f;
	pipelineBuilder._viewport.width = (float)window.width;
	pipelineBuilder._viewport.height = (float)window.height;
	pipelineBuilder._viewport.minDepth = 0.0f;
	pipelineBuilder._viewport.maxDepth = 1.0f;

	pipelineBuilder._scissor.offset = { 0, 0 };
	pipelineBuilder._scissor.extent = {(uint32_t)window.width, (uint32_t)window.height};

	//configure the rasterizer to draw filled triangles
	pipelineBuilder._rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);

	//we dont use multisampling, so just run the default one
	pipelineBuilder._multisampling = vkinit::multisampling_state_create_info();

	//a single blend attachment with no blending and writing to RGBA
	pipelineBuilder._colorBlendAttachment = vkinit::color_blend_attachment_state();


	//default depthtesting
	pipelineBuilder._depthStencil = vkinit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	// //build the mesh pipeline
	// VertexInputDescription vertexDescription = Vertex::get_vertex_description();

	// //connect the pipeline builder vertex input info to the one we get from Vertex
	// pipelineBuilder._vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
	// pipelineBuilder._vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

	// pipelineBuilder._vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
	// pipelineBuilder._vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

	return pipelineBuilder;
}

std::unordered_map<std::string, VkShaderModule> loadShaders(VkDevice _device)
{
	std::unordered_map<std::string, VkShaderModule> shaders;

	VkShaderModule colorMeshShader;
	if (!VulkanEngine::load_shader_module(_device, "shaders/default_lit.frag.spv", &colorMeshShader))
	{
		std::cout << "Error when building the colored mesh shader" << std::endl;
	}
	else 
	{
		shaders.emplace("shaders/default_lit.frag.spv", colorMeshShader);
	}

	VkShaderModule texturedMeshShader;
	if (!VulkanEngine::load_shader_module(_device, "shaders/textured_lit.frag.spv", &texturedMeshShader))
	{
		std::cout << "Error when building the colored mesh shader" << std::endl;
	}
	else 
	{
		shaders.emplace("shaders/textured_lit.frag.spv", texturedMeshShader);
	}

	VkShaderModule meshVertShader;
	if (!VulkanEngine::load_shader_module(_device, "shaders/tri_mesh_ssbo.vert.spv", &meshVertShader))
	{
		std::cout << "Error when building the mesh vertex shader module" << std::endl;
	}
	else 
	{
		shaders.emplace("shaders/tri_mesh_ssbo.vert.spv", meshVertShader);
	}

	return shaders;
}

void DefaultMaterial::makePipelineLayout(VkDevice _device, VkDescriptorSetLayout _globalSetLayout, VkDescriptorSetLayout _objectSetLayout)
{
	//we start from just the default empty pipeline layout info
	VkPipelineLayoutCreateInfo mesh_pipeline_layout_info = vkinit::pipeline_layout_create_info();

	//setup push constants
	VkPushConstantRange push_constant;
	//offset 0
	push_constant.offset = 0;
	//size of a MeshPushConstant struct
	push_constant.size = sizeof(MeshPushConstants);
	//for the vertex shader
	push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	mesh_pipeline_layout_info.pPushConstantRanges = &push_constant;
	mesh_pipeline_layout_info.pushConstantRangeCount = 1;

	VkDescriptorSetLayout setLayouts[] = { _globalSetLayout, _objectSetLayout };

	mesh_pipeline_layout_info.setLayoutCount = 2;
	mesh_pipeline_layout_info.pSetLayouts = setLayouts;

	VK_CHECK(vkCreatePipelineLayout(_device, &mesh_pipeline_layout_info, nullptr, &pipelineLayout));
}

void DefaultMaterial::makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules)
{
	VertexInputDescription vertexDescription = Vertex::get_vertex_description();
	pipelineBuilder._vertexInputInfo = vertexDescription.toVertexInputInfo();

	//hook the push constants layout
	pipelineBuilder._pipelineLayout = pipelineLayout;
	
	//build the mesh triangle pipeline
	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, shaderModules.at("shaders/tri_mesh_ssbo.vert.spv")));

	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, shaderModules.at("shaders/default_lit.frag.spv")));

	pipeline = pipelineBuilder.build_pipeline(_device, _renderPass);
}

void TexturedMaterial::makePipelineLayout(VkDevice _device, VkDescriptorSetLayout _globalSetLayout, VkDescriptorSetLayout _objectSetLayout)
{
	//we start from  the normal mesh layout
	VkPipelineLayoutCreateInfo textured_pipeline_layout_info = vkinit::pipeline_layout_create_info(); // mesh_pipeline_layout_info;

	VkPushConstantRange push_constant;
	//offset 0
	push_constant.offset = 0;
	//size of a MeshPushConstant struct
	push_constant.size = sizeof(MeshPushConstants);
	//for the vertex shader
	push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	textured_pipeline_layout_info.pPushConstantRanges = &push_constant;
	textured_pipeline_layout_info.pushConstantRangeCount = 1;
		
	VkDescriptorSetLayout texturedSetLayouts[] = { _globalSetLayout, _objectSetLayout, shaderLayout /*texturedShaderUniforms.shaderLayout*/ };

	textured_pipeline_layout_info.setLayoutCount = 3;
	textured_pipeline_layout_info.pSetLayouts = texturedSetLayouts;

	VK_CHECK(vkCreatePipelineLayout(_device, &textured_pipeline_layout_info, nullptr, &pipelineLayout));
}

void TexturedMaterial::makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules)
{
	VertexInputDescription vertexDescription = Vertex::get_vertex_description();
	pipelineBuilder._vertexInputInfo = vertexDescription.toVertexInputInfo();

	// pipelineBuilder._shaderStages.clear();
	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, shaderModules.at("shaders/tri_mesh_ssbo.vert.spv")));

	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, shaderModules.at("shaders/textured_lit.frag.spv")));

	pipelineBuilder._pipelineLayout = mat->pipelineLayout;
	mat->pipeline = pipelineBuilder.build_pipeline(_device, _renderPass);
}

void VulkanEngine::init_pipelines()
{
	std::unordered_map<std::string, VkShaderModule> shaderModules = loadShaders(_device);
	
	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
	PipelineBuilder pipelineBuilder = Material::getPipelineBuilder(*_window);

	{
		Material* mat1 = resourceManager.get_material("defaultmesh");
		if (mat1 == nullptr)
		{
			mat1 = resourceManager.create_material("defaultmesh");
		}

		// Copy mat
		DefaultMaterial defaultMat;
		defaultMat.shaderLayout = mat1->shaderLayout;
		defaultMat.matDescriptors = mat1->matDescriptors;

		defaultMat.makePipelineLayout(_device, _globalSetLayout, _objectSetLayout);
		defaultMat.makePipeline(_device, _renderPass, pipelineBuilder, shaderModules);

		*mat1 = defaultMat;
	}

	{
		Material* mat = resourceManager.get_material("texturedmesh");
		if (mat == nullptr)
		{
			mat = resourceManager.create_material("texturedmesh");
		}

		// Copy mat
		TexturedMaterial texturedMat;
		texturedMat.shaderLayout = mat->shaderLayout;
		texturedMat.matDescriptors = mat->matDescriptors;

		texturedMat.makePipelineLayout(_device, _globalSetLayout, _objectSetLayout);
		texturedMat.makePipeline(_device, _renderPass, pipelineBuilder, shaderModules);

		mat->pipelineLayout = texturedMat.pipelineLayout;
		mat->pipeline = texturedMat.pipeline;
	}

	for (auto& shaderModule : shaderModules)
	{
		vkDestroyShaderModule(_device, shaderModule.second, nullptr);
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
bool VulkanEngine::load_shader_module(VkDevice _device, const char* filePath, VkShaderModule* outShaderModule)
{
	auto shaderCode = readFile(filePath);
	vk::ShaderModule fShaderModule (_device, shaderCode);
	
	*outShaderModule = fShaderModule.shaderModule;
	return true;
}

VkPipeline PipelineBuilder::build_pipeline(VkDevice device, VkRenderPass pass)
{
	//make viewport state from our stored viewport and scissor.
		//at the moment we wont support multiple viewports or scissors
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;

	viewportState.viewportCount = 1;
	viewportState.pViewports = &_viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &_scissor;

	//setup dummy color blending. We arent using transparent objects yet
	//the blending is just "no blend", but we do write to the color attachment
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext = nullptr;

	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &_colorBlendAttachment;

	//build the actual pipeline
	//we now use all of the info structs we have been writing into into this one to create the pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;

	pipelineInfo.stageCount = _shaderStages.size();
	pipelineInfo.pStages = _shaderStages.data();
	pipelineInfo.pVertexInputState = &_vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &_inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &_rasterizer;
	pipelineInfo.pMultisampleState = &_multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDepthStencilState = &_depthStencil;
	pipelineInfo.layout = _pipelineLayout;
	pipelineInfo.renderPass = pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	//its easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
	VkPipeline newPipeline;
	if (vkCreateGraphicsPipelines(
		device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) {
		std::cout << "failed to create pipline\n";
		return VK_NULL_HANDLE; // failed to create graphics pipeline
	}
	else
	{
		return newPipeline;
	}
}

void VulkanEngine::addMesh(CPUMesh& mesh, const std::string& key)
{
	GPUMesh gpuMesh = uploadMesh(mesh);
	resourceManager._meshes[key] = gpuMesh;
}

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void CPUTexture::loadFromFile(const char* path)
{
	pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);	

	if (!pixels) 
	{
		throw std::exception((std::string("Failed to load texture file : ") + path).c_str());
	}
}

void CPUTexture::destroy()
{
	stbi_image_free(pixels);
}

GPUTexture VulkanEngine::uploadTexture(const CPUTexture& cpuTexture)
{
	void* pixel_ptr = cpuTexture.pixels;
	VkDeviceSize imageSize = cpuTexture.texWidth * cpuTexture.texHeight * 4;

	VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;

	AllocatedBuffer stagingBuffer = create_buffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

	void* data;
	vmaMapMemory(_allocator, stagingBuffer._allocation, &data);

	memcpy(data, pixel_ptr, static_cast<size_t>(imageSize));

	vmaUnmapMemory(_allocator, stagingBuffer._allocation);

	VkExtent3D imageExtent;
	imageExtent.width = static_cast<uint32_t>(cpuTexture.texWidth);
	imageExtent.height = static_cast<uint32_t>(cpuTexture.texHeight);
	imageExtent.depth = 1;
	
	VkImageCreateInfo dimg_info = vkinit::image_create_info(image_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

	// AllocatedImage newImage;	
	GPUTexture newImage;
	
	VmaAllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	//allocate and create the image
	vmaCreateImage(_allocator, &dimg_info, &dimg_allocinfo, &newImage.image._image, &newImage.image._allocation, nullptr);
	
	//transition image to transfer-receiver	
	immediate_submit([&](VkCommandBuffer cmd) {
		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		VkImageMemoryBarrier imageBarrier_toTransfer = {};
		imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

		imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier_toTransfer.image = newImage.image._image;
		imageBarrier_toTransfer.subresourceRange = range;

		imageBarrier_toTransfer.srcAccessMask = 0;
		imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		//barrier the image into the transfer-receive layout
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);
		
		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = imageExtent;

		//copy the buffer into the image
		vkCmdCopyBufferToImage(cmd, stagingBuffer._buffer, newImage.image._image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

		imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		
		imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		//barrier the image into the shader readable layout
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
	});

	vmaDestroyBuffer(_allocator, stagingBuffer._buffer, stagingBuffer._allocation);

	return newImage;
}

void VulkanEngine::load_images()
{
	// GPUTexture lostEmpire;
	
	// vkutil::load_image_from_file(*this, "assets/lost_empire-RGBA.png", lostEmpire.image);

	CPUTexture cpuTexture;
	cpuTexture.loadFromFile("assets/lost_empire-RGBA.png");

	GPUTexture gpuTexture = uploadTexture(cpuTexture);
	cpuTexture.destroy();
	// gpuTexture.makeView();
	
	VkImageViewCreateInfo imageinfo = vkinit::imageview_create_info(VK_FORMAT_R8G8B8A8_SRGB, gpuTexture.image._image, VK_IMAGE_ASPECT_COLOR_BIT);
	vkCreateImageView(_device, &imageinfo, nullptr, &gpuTexture.imageView);

	resourceManager._loadedTextures["empire_diffuse"] = gpuTexture;
}

GPUMesh VulkanEngine::uploadMesh(CPUMesh& mesh)
{
	const size_t bufferSize= mesh._vertices.size() * sizeof(Vertex);
	//allocate vertex buffer
	VkBufferCreateInfo stagingBufferInfo = {};
	stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferInfo.pNext = nullptr;
	//this is the total size, in bytes, of the buffer we are allocating
	stagingBufferInfo.size = bufferSize;
	//this buffer is going to be used as a Vertex Buffer
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;


	//let the VMA library know that this data should be writeable by CPU, but also readable by GPU
	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

	AllocatedBuffer stagingBuffer;

	//allocate the buffer
	VK_CHECK(vmaCreateBuffer(_allocator, &stagingBufferInfo, &vmaallocInfo,
		&stagingBuffer._buffer,
		&stagingBuffer._allocation,
		nullptr));	

	//copy vertex data
	void* data;
	vmaMapMemory(_allocator, stagingBuffer._allocation, &data);

	memcpy(data, mesh._vertices.data(), mesh._vertices.size() * sizeof(Vertex));

	vmaUnmapMemory(_allocator, stagingBuffer._allocation);


	//allocate vertex buffer
	VkBufferCreateInfo vertexBufferInfo = {};
	vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferInfo.pNext = nullptr;
	//this is the total size, in bytes, of the buffer we are allocating
	vertexBufferInfo.size = bufferSize;
	//this buffer is going to be used as a Vertex Buffer
	vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	//let the VMA library know that this data should be gpu native	
	vmaallocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	GPUMesh gpuMesh;

	//allocate the buffer
	VK_CHECK(vmaCreateBuffer(_allocator, &vertexBufferInfo, &vmaallocInfo,
		&gpuMesh._vertexBuffer._buffer,
		&gpuMesh._vertexBuffer._allocation,
		nullptr));
	// //add the destruction of triangle mesh buffer to the deletion queue
	// _mainDeletionQueue.push_function([=]() {

	// 	vmaDestroyBuffer(_allocator, mesh._vertexBuffer._buffer, mesh._vertexBuffer._allocation);
	// });

	immediate_submit([=](VkCommandBuffer cmd) {
		VkBufferCopy copy;
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = bufferSize;
		vkCmdCopyBuffer(cmd, stagingBuffer._buffer, gpuMesh._vertexBuffer._buffer, 1, & copy);
	});

	vmaDestroyBuffer(_allocator, stagingBuffer._buffer, stagingBuffer._allocation);

	gpuMesh.size = mesh._vertices.size();

	return gpuMesh;
}


Material* ResourceManager::create_material(const std::string& name)
{
	Material mat;
	_materials[name] = mat;
	return &_materials[name];
}

Material* ResourceManager::get_material(const std::string& name)
{
	//search for the object, and return nullpointer if not found
	auto it = _materials.find(name);
	if (it == _materials.end()) {
		return nullptr;
	}
	else {
		return &(*it).second;
	}
}


GPUMesh* ResourceManager::get_mesh(const std::string& name)
{
	auto it = _meshes.find(name);
	if (it == _meshes.end()) {
		return nullptr;
	}
	else {
		return &(*it).second;
	}
}


void VulkanEngine::draw_objects(VkCommandBuffer cmd,RenderObject* first, int count)
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
	vmaMapMemory(_allocator, get_current_frame().cameraBuffer._allocation, &data);

	memcpy(data, &camData, sizeof(GPUCameraData));

	vmaUnmapMemory(_allocator, get_current_frame().cameraBuffer._allocation);

	// Test Material Update
	{
		if (Material* mat = resourceManager.get_material("texturedmesh"))
		{
			struct TestStruct
			{
				float multValue[4];
			};

			TestStruct testData;
			for (int i = 0; i < 4; i++)
				testData.multValue[i] = 0.1;

			void* data;
			
			AllocatedBuffer& buffer = mat->uniformBuffers[get_current_frame_id()]; 
			vmaMapMemory(_allocator, buffer._allocation, &data);

			memcpy(data, &testData, sizeof(TestStruct));

			vmaUnmapMemory(_allocator, buffer._allocation);
		}
	}



	float framed = (_frameNumber / 120.f);

	_sceneParameters.ambientColor = { sin(framed),0,cos(framed),1 };

	char* sceneData;
	vmaMapMemory(_allocator, _sceneParameterBuffer._allocation , (void**)&sceneData);

	int frameIndex = _frameNumber % FRAME_OVERLAP;

	sceneData += pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameIndex;

	memcpy(sceneData, &_sceneParameters, sizeof(GPUSceneData));

	vmaUnmapMemory(_allocator, _sceneParameterBuffer._allocation);


	void* objectData;
	vmaMapMemory(_allocator, get_current_frame().objectBuffer._allocation, &objectData);
	
	GPUObjectData* objectSSBO = (GPUObjectData*)objectData;
	
	for (int i = 0; i < count; i++)
	{
		RenderObject& object = first[i];
		objectSSBO[i].modelMatrix = object.transformMatrix;
	}
	
	vmaUnmapMemory(_allocator, get_current_frame().objectBuffer._allocation);

	GPUMesh* lastMesh = nullptr;
	Material* lastMaterial = nullptr;
	
	for (int i = 0; i < count; i++)
	{
		RenderObject& object = first[i];

		//only bind the pipeline if it doesnt match with the already bound one
		if (object.material != lastMaterial) {

			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
			lastMaterial = object.material;

			uint32_t uniform_offset = pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameIndex;
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 0, 1, &get_current_frame().globalDescriptor, 1, &uniform_offset);
		
			//object data descriptor
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 1, 1, &get_current_frame().objectDescriptor, 0, nullptr);

			if (object.material->matDescriptors != VK_NULL_HANDLE) {
				//texture descriptor
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 2, 1, &object.material->matDescriptors, 0, nullptr);

			}
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
		vkCmdDraw(cmd, object.mesh->size, 1,0 , i);
	}
}

AllocatedBuffer VulkanEngine::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	//allocate vertex buffer
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = allocSize;

	bufferInfo.usage = usage;


	//let the VMA library know that this data should be writeable by CPU, but also readable by GPU
	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = memoryUsage;

	AllocatedBuffer newBuffer;

	//allocate the buffer
	VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo,
		&newBuffer._buffer,
		&newBuffer._allocation,
		nullptr));

	return newBuffer;
}

size_t VulkanEngine::pad_uniform_buffer_size(size_t originalSize)
{
	// Calculate required alignment based on minimum device offset alignment
	size_t minUboAlignment = _gpuProperties.limits.minUniformBufferOffsetAlignment;
	size_t alignedSize = originalSize;
	if (minUboAlignment > 0) {
		alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}
	return alignedSize;
}


void VulkanEngine::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	VkCommandBuffer cmd;

	//allocate the default command buffer that we will use for rendering
	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_uploadContext._commandPool, 1);

	VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &cmd));

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));


	function(cmd);


	VK_CHECK(vkEndCommandBuffer(cmd));

	VkSubmitInfo submit = vkinit::submit_info(&cmd);


	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, _uploadContext._uploadFence));

	vkWaitForFences(_device, 1, &_uploadContext._uploadFence, true, 9999999999);
	vkResetFences(_device, 1, &_uploadContext._uploadFence);

	vkResetCommandPool(_device, _uploadContext._commandPool, 0);
}

void VulkanEngine::init_descriptors()
{

	//create a descriptor pool that will hold 10 uniform buffers
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
	
	vkCreateDescriptorPool(_device, &pool_info, nullptr, &_descriptorPool);	
	
	VkDescriptorSetLayoutBinding cameraBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);
	VkDescriptorSetLayoutBinding sceneBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	
	VkDescriptorSetLayoutBinding bindings[] = { cameraBind,sceneBind };

	VkDescriptorSetLayoutCreateInfo setinfo = {};
	setinfo.bindingCount = 2;
	setinfo.flags = 0;
	setinfo.pNext = nullptr;
	setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setinfo.pBindings = bindings;

	vkCreateDescriptorSetLayout(_device, &setinfo, nullptr, &_globalSetLayout);

	VkDescriptorSetLayoutBinding objectBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

	VkDescriptorSetLayoutCreateInfo set2info = {};
	set2info.bindingCount = 1;
	set2info.flags = 0;
	set2info.pNext = nullptr;
	set2info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set2info.pBindings = &objectBind;

	vkCreateDescriptorSetLayout(_device, &set2info, nullptr, &_objectSetLayout);

	// Test Material Create Layout
	{
		// VkDescriptorSetLayoutBinding textureBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
		// VkDescriptorSetLayoutBinding testBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);

		// VkDescriptorSetLayoutBinding  t[2] = {textureBind, testBind};

		// VkDescriptorSetLayoutCreateInfo set3info = {};
		// set3info.bindingCount = 2;
		// set3info.flags = 0;
		// set3info.pNext = nullptr;
		// set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		// set3info.pBindings = t;

		// vkCreateDescriptorSetLayout(_device, &set3info, nullptr, &_singleTextureSetLayout);

		Material* mat = resourceManager.get_material("texturedmesh");
		if (mat == nullptr)
		{
			mat = resourceManager.create_material("texturedmesh");
		}

		mat->createLayout(_device);
		// texturedShaderUniforms.createLayout(_device);
	}


	const size_t sceneParamBufferSize = FRAME_OVERLAP * pad_uniform_buffer_size(sizeof(GPUSceneData));

	_sceneParameterBuffer = create_buffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	

	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		_frames[i].cameraBuffer = create_buffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		const int MAX_OBJECTS = 10000;
		_frames[i].objectBuffer = create_buffer(sizeof(GPUObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.pNext = nullptr;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = _descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &_globalSetLayout;

		vkAllocateDescriptorSets(_device, &allocInfo, &_frames[i].globalDescriptor);

		VkDescriptorSetAllocateInfo objectSetAlloc = {};
		objectSetAlloc.pNext = nullptr;
		objectSetAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		objectSetAlloc.descriptorPool = _descriptorPool;
		objectSetAlloc.descriptorSetCount = 1;
		objectSetAlloc.pSetLayouts = &_objectSetLayout;

		vkAllocateDescriptorSets(_device, &objectSetAlloc, &_frames[i].objectDescriptor);

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

		vkUpdateDescriptorSets(_device, 3, setWrites, 0, nullptr);
	}
}
