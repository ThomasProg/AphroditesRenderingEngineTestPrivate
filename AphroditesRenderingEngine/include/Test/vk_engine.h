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
#include "Test/ResourceManager.hpp"
#include "Passes/PassQueue.hpp"

struct MeshPushConstants 
{
	glm::vec4 data;
	glm::mat4 render_matrix;
};

struct RenderObject 
{
	GPUMesh* mesh;

	GraphicsMaterial* material;

	glm::mat4 transformMatrix;
};

struct FrameData 
{
	VkSemaphore _presentSemaphore, _renderSemaphore;
	VkFence _renderFence;

	VkCommandPool _commandPool;
	VkCommandBuffer _mainCommandBuffer;

	AllocatedBuffer cameraBuffer;
	VkDescriptorSet globalDescriptor;

	AllocatedBuffer objectBuffer;
	VkDescriptorSet objectDescriptor;
};

struct GPUCameraData
{
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;
};

struct GPUSceneData 
{
	glm::vec4 fogColor; // w is for exponent
	glm::vec4 fogDistances; //x for min, y for max, zw unused.
	glm::vec4 ambientColor;
	glm::vec4 sunlightDirection; //w for sun power
	glm::vec4 sunlightColor;
};

struct GPUObjectData 
{
	glm::mat4 modelMatrix;
};

constexpr unsigned int FRAME_OVERLAP = 2; // 2


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
	std::vector<VkImageView> _colorImageViews;

	// Depth Image
	VkFormat _depthImageFormat;
	AllocatedImage _depthImage;
	VkImageView _depthImageView;

	void init(VkDevice device);
};

class VulkanEngineBase
{
public:
	VkRenderPass _renderPass;
	
public:
	class VulkanContext* context = nullptr;

protected:
	VkRenderPass init_default_renderpass(VkFormat _colorImageFormat, VkFormat _depthImageFormat, VkImageLayout finalLayout);

	void destroyRenderpass();

	// void recordCommand(VkCommandBuffer cmd);
	// virtual void recordRenderPasses();
};

// Surface Support
// Swapchain Support
class VulkanEngineSurfaceSupport : public VulkanEngineBase
{

};

class VulkanEngine : public VulkanEngineSurfaceSupport
{
public:
	PassQueue passQueue;
	ResourceManager resourceManager;

	bool _isInitialized{ false };
	int _frameNumber {0};

	class Window* _window{ nullptr };

	FrameData _frames[FRAME_OVERLAP];
	
	// VkRenderPass _renderPass;

	VkSurfaceKHR _surface;
	SwapChainData swapChainData;

	std::vector<VkFramebuffer> _framebuffers;	

	VkDescriptorPool _descriptorPool;

	VkDescriptorSetLayout _globalSetLayout;
	VkDescriptorSetLayout _objectSetLayout;

	GPUSceneData _sceneParameters;
	AllocatedBuffer _sceneParameterBuffer;

public:
	//initializes everything in the engine
	void init(Window* window);

	//shuts down the engine
	void cleanup();

public:
	//draw loop
	virtual void draw();
	
	int get_current_frame_id();
	FrameData& get_current_frame();
	FrameData& get_last_frame();

protected:
public:
	void recordColorRenderPass(VkCommandBuffer cmd, VkRenderPass _renderPass, VkFramebuffer targetFramebuffer);
	virtual void recordRenderPasses(VkFramebuffer targetFramebuffer);

private:
	//default array of renderable objects
	std::vector<RenderObject> _renderables;
	
	//our draw function
	void draw_objects(VkCommandBuffer cmd, RenderObject* first, int count);

protected:
	void recreateSwapchain();

public:
	virtual void createFullSwapchain();
protected:
	virtual void destroyFullSwapchain();

private:
	void init_vulkan();

	static SwapChainData init_swapchain(VkSurfaceKHR _surface, PhysicalDevice _chosenGPU, class Window* _window, VkDevice _device, VmaAllocator& _allocator);

	void init_framebuffers();

	void init_commandPool();
	void init_commands();

	void init_sync_structures();

	void init_pipelines();

public:
	void init_descriptors();

public:
	void addShaderModule(const std::string& path);

	void addRenderObject(const RenderObject& renderObject)
	{
		_renderables.emplace_back(renderObject);
	}
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

	//loads a shader module from a spir-v file. Returns false if it errors
	static bool load_shader_module(VkDevice _device, const char* filePath, VkShaderModule* outShaderModule);

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