#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vk_types.h>

#include <functional>

#include "PhysicalDevice.hpp"

// Manages the memory of Vulkan buffers
class MemoryManager
{
public:
#pragma region Fields
	VmaAllocator _allocator; //vma lib allocator

	VkPhysicalDeviceProperties _gpuProperties;
    PhysicalDevice _chosenGPU;
	VkDevice _device;

	VkQueue _graphicsQueue;
	VkQueue _computeQueue;

	QueueFamilyIndices queueFamilies;

	// for immediate submits
	UploadContext _uploadContext;

	int nextBufferID = 1;

#pragma endregion


#pragma region InitDestroy
	void init(VkInstance instance, VkSurfaceKHR& _surface);
	void destroy();
#pragma endregion

#pragma region Utilities
    static size_t pad_uniform_buffer_size(size_t originalSize, VkDeviceSize minUniformBufferOffsetAlignment);
	size_t pad_uniform_buffer_size(size_t originalSize);

	void createImmediateSubmitCommandPool();
	void destroyImmediateSubmitCommandPool();
    void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
	void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function, VkSubmitInfo submitInfo);
#pragma endregion

#pragma region BufferCreation
    AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
    void destroyBuffer(AllocatedBuffer& buffer);

	struct GPUMesh uploadMesh(struct CPUMesh& mesh);
	struct AllocatedImage uploadTexture(const struct CPUTexture& cpuTexture);
	struct GPUTexture makeGPUTexture(struct AllocatedImage& newImage, VkImageViewCreateInfo& imageViewInfo);
#pragma endregion

#pragma region PickDeviceUtilities
	static VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
    static bool isDeviceSuitable(QueueFamilyIndices indices, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
#pragma endregion


	void changeImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
	void changeImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);
	void changeImageLayoutFullBarrier(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

	VkResult createSemaphore(VkSemaphore& semaphore);
	void destroySemaphore(VkSemaphore& semaphore);

	VkRenderPass init_default_renderpass(VkFormat _colorImageFormat, VkFormat _depthImageFormat, VkImageLayout finalLayout);

	VkResult createAllocatedImage(AllocatedImage& image, VkFormat imageFormat, const VkExtent3D& imageExtent, VkImageUsageFlags usageFlags);
	VkResult createAllocatedImage2D(AllocatedImage& image, VkFormat imageFormat, VkImageUsageFlags usageFlags, uint32_t width, uint32_t height);
	VkResult createAllocatedDepthImage2D(AllocatedImage& depthImage, VkFormat imageFormat, uint32_t width, uint32_t height);

	void destroyAllocatedImage(AllocatedImage& image);	
};