// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>
#include <vector>
#include <functional>
#include <deque>
#include <vk_mesh.h>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "Vk/Instance.hpp"
#include "PhysicalDevice.hpp"
#include "Material.hpp"
#include "MemoryManager.hpp"
// #include "Test/ResourceManager.hpp"
#include "Passes/PassQueue.hpp"

#include "RenderObject.hpp"

struct MeshPushConstants 
{
	glm::vec4 data;
	glm::mat4 render_matrix;
};

struct FrameData 
{
	VkSemaphore _presentSemaphore, _renderSemaphore;
	VkFence _renderFence;

	VkCommandPool _commandPool;
	VkCommandBuffer _mainCommandBuffer;

	// AllocatedBuffer cameraBuffer;
	// VkDescriptorSet globalDescriptor;

	// AllocatedBuffer objectBuffer;
	// VkDescriptorSet objectDescriptor;
};

struct GPUCameraData
{
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;
};

struct GPUObjectData 
{
	glm::mat4 modelMatrix;
};

constexpr unsigned int FRAME_OVERLAP = 1; // 2


struct VulkanCore
{
	vk::Instance instance;
	VkDebugUtilsMessengerEXT _debug_messenger;

	void init();
	void destroy();

	operator vk::Instance()
	{
		return instance;
	}
};

struct SwapChainData
{
	VkSwapchainKHR _swapchain;

	// Color Images
	VkFormat _colorImageFormat;
	std::vector<VkImage> _colorImages;
	// std::vector<VkImageView> _colorImageViews;

	// Depth Image
	VkFormat _depthImageFormat;
	// AllocatedImage _depthImage;
	VkImageView _depthImageView;

	void init(VkDevice device);
};

class VulkanEngineBase
{
public:
	class VulkanContext* context = nullptr;

protected:
	VkRenderPass init_default_renderpass(VkFormat _colorImageFormat, VkFormat _depthImageFormat, VkImageLayout finalLayout);
};

// Surface Support
// Swapchain Support
class VulkanEngineSurfaceSupport : public VulkanEngineBase
{

};

class VulkanEngine : public VulkanEngineSurfaceSupport
{
public:
	// PassQueue passQueue;
	// ResourceManager resourceManager;

	bool _isInitialized{ false };
	int _frameNumber {0};

	class Window* _window{ nullptr };

	FrameData _frames[FRAME_OVERLAP];
	
	// VkRenderPass renderPass;
	// VkRenderPass _renderPass;

	VkSurfaceKHR _surface;
	SwapChainData swapChainData;

	// std::vector<VkFramebuffer> _framebuffers;	

	// VkDescriptorPool _descriptorPool;

	// VkDescriptorSetLayout _globalSetLayout;
	// VkDescriptorSetLayout _objectSetLayout;

	// // GPUSceneData _sceneParameters;
	// AllocatedBuffer _sceneParameterBuffer;

public:
	//initializes everything in the engine
	void init(Window* window);

	//shuts down the engine
	void cleanup();

    virtual void firstSetup(class MemoryManager& memoryManager) {}
    virtual void lastDestroy(class MemoryManager& memoryManager) {}

public:
	//draw loop
	virtual void draw();
	
	int get_current_frame_id();
	FrameData& get_current_frame();
	FrameData& get_last_frame();

protected:
	void recreateSwapchain();

public:
	virtual void createFullSwapchain();
protected:
	virtual void destroyFullSwapchain();

private:
public:
	void init_vulkan();

	static SwapChainData init_swapchain(VkSurfaceKHR _surface, PhysicalDevice _chosenGPU, class Window* _window, VkDevice _device, VmaAllocator& _allocator);

	void init_commandPool();
	void init_commands();

	void init_sync_structures();
};

class VulkanContext
{
public:
	MemoryManager* memoryManager = new MemoryManager();
	// VulkanEngine engine;
	VulkanCore core;

	VulkanContext()
	{
		// engine.context = this;
		// engine.resourceManager.memoryManager = memoryManager;
	}

	//initializes everything in the engine
	void init(Window* window)
	{
		core.init();
		// engine.init(window);
	}

	//shuts down the engine
	void cleanup()
	{
		// engine.cleanup();
		memoryManager->destroy();
		delete memoryManager;
		core.destroy();
	}
};