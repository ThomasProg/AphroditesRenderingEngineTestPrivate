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

class PipelineBuilder 
{
public:

	std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
	VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
	VkViewport _viewport;
	VkRect2D _scissor;
	VkPipelineRasterizationStateCreateInfo _rasterizer;
	VkPipelineColorBlendAttachmentState _colorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo _multisampling;
	VkPipelineLayout _pipelineLayout;
	VkPipelineDepthStencilStateCreateInfo _depthStencil;
	VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
};



struct DeletionQueue
{
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()>&& function) {
        deletors.push_back(function);
    }

    void flush() {
        // reverse iterate the deletion queue to execute all the functions
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
            (*it)(); //call functors
        }

        deletors.clear();
    }
};

struct MeshPushConstants 
{
	glm::vec4 data;
	glm::mat4 render_matrix;
};


struct Material 
{
	// Pipeline and its layout
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;

	// Contains textures' and uniforms' descriptor sets
	VkDescriptorSetLayout shaderLayout = VK_NULL_HANDLE;
	VkDescriptorSet matDescriptors{VK_NULL_HANDLE};

	void createLayout(VkDevice device);

	void destroyLayout(VkDevice device)
	{
		vkDestroyDescriptorSetLayout(device, shaderLayout, nullptr);
	}

	// uniform buffer for each frame
	std::vector<AllocatedBuffer> uniformBuffers;

	// virtual MakeLayout();

	void destroyPipeline(VkDevice _device);

	static PipelineBuilder getPipelineBuilder(class Window& window);

	virtual void makePipelineLayout(VkDevice _device, VkDescriptorSetLayout _globalSetLayout, VkDescriptorSetLayout _objectSetLayout) {}
	virtual void makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules) {}
};

struct DefaultMaterial : public Material
{
	virtual void makePipelineLayout(VkDevice _device, VkDescriptorSetLayout _globalSetLayout, VkDescriptorSetLayout _objectSetLayout);
	virtual void makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules);
};

struct TexturedMaterial : public Material
{
	virtual void makePipelineLayout(VkDevice _device, VkDescriptorSetLayout _globalSetLayout, VkDescriptorSetLayout _objectSetLayout);
	virtual void makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules);
};

struct CPUTexture 
{
	unsigned char* pixels;
	int texWidth, texHeight, texChannels;

	void loadFromFile(const char* path);
	void destroy();
};

struct GPUTexture 
{
	AllocatedImage image;
	VkImageView imageView;
};

struct RenderObject 
{
	GPUMesh* mesh;

	Material* material;

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


	// AllocatedBuffer testBuffer;
};

// Test
class TexturedShaderUniforms
{
public:
	// VkDescriptorSetLayout shaderLayout = VK_NULL_HANDLE;
	// std::unique_ptr<AllocatedBuffer> uniformBuffers; // size = FRAME_OVERLAP

	// void createLayout(VkDevice device);

	// void destroy(VkDevice device)
	// {
	// 	vkDestroyDescriptorSetLayout(device, shaderLayout, nullptr);
	// }
};

struct UploadContext 
{
	VkFence _uploadFence;
	VkCommandPool _commandPool;	
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

constexpr unsigned int FRAME_OVERLAP = 2;


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
};

struct ResourceManager
{
	std::unordered_map<std::string, Material> _materials;
	std::unordered_map<std::string, GPUMesh> _meshes;
	std::unordered_map<std::string, GPUTexture> _loadedTextures;
	
	//create material and add it to the map
	Material* create_material(const std::string& name);

	//returns nullptr if it cant be found
	Material* get_material(const std::string& name);

	//returns nullptr if it cant be found
	GPUMesh* get_mesh(const std::string& name);

	void clear(VkDevice _device, VmaAllocator _allocator);
};

class VulkanEngine 
{
public:
	ResourceManager resourceManager;

	bool _isInitialized{ false };
	int _frameNumber {0};
	int _selectedShader{ 0 };

	VkSampler blockySampler;

	// VkPipelineLayout meshPipLayout;
	// VkPipelineLayout texturedPipeLayout;
	// VkPipeline meshPipeline;
	// VkPipeline texPipeline;

	// VkExtent2D _windowExtent{ 1700 , 900 };

	class Window* _window{ nullptr };

	PhysicalDevice _chosenGPU;
	VkDevice _device;
	VkPhysicalDeviceProperties _gpuProperties;

	VkQueue _graphicsQueue;
	uint32_t _graphicsQueueFamily;

	VulkanCore core;

	FrameData _frames[FRAME_OVERLAP];
	
	VkRenderPass _renderPass;

	VkSurfaceKHR _surface;
	SwapChainData swapChainData;

	std::vector<VkFramebuffer> _framebuffers;	

    // DeletionQueue _mainDeletionQueue;
	
	VmaAllocator _allocator; //vma lib allocator

	VkDescriptorPool _descriptorPool;

	VkDescriptorSetLayout _globalSetLayout;
	VkDescriptorSetLayout _objectSetLayout;
	// VkDescriptorSetLayout _singleTextureSetLayout;
	TexturedShaderUniforms texturedShaderUniforms;

	GPUSceneData _sceneParameters;
	AllocatedBuffer _sceneParameterBuffer;

	UploadContext _uploadContext;
	//initializes everything in the engine
	void init(Window* window);

	//shuts down the engine
	void cleanup();

	//draw loop
	void draw();

	//run main loop
	void run();
	
	int get_current_frame_id();
	FrameData& get_current_frame();
	FrameData& get_last_frame();

	void addRenderObject(const RenderObject& renderObject)
	{
		_renderables.emplace_back(renderObject);
	}
	
	void addMesh(CPUMesh& mesh, const std::string& key);

	//default array of renderable objects
	std::vector<RenderObject> _renderables;

	//our draw function
	void draw_objects(VkCommandBuffer cmd, RenderObject* first, int count);

	GPUTexture uploadTexture(const CPUTexture& cpuTexture);
	AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	size_t pad_uniform_buffer_size(size_t originalSize);

	void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
private:
	void recreateSwapchain();
	void createFullSwapchain();
	void destroyFullSwapchain();

	void init_vulkan();

	static SwapChainData init_swapchain(VkSurfaceKHR _surface, PhysicalDevice _chosenGPU, class Window* _window, VkDevice _device, VmaAllocator& _allocator);

	void init_default_renderpass();

	void init_framebuffers();

	void init_commands();

	void init_sync_structures();

	void init_pipelines();

	void init_descriptors();

public:
	//loads a shader module from a spir-v file. Returns false if it errors
	static bool load_shader_module(VkDevice _device, const char* filePath, VkShaderModule* outShaderModule);

	// void load_meshes();

	void load_images();

	GPUMesh uploadMesh(CPUMesh& mesh);
};
