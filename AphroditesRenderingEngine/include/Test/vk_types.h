// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

//we want to immediately abort when there is an error. In normal engines this would give an error message to the user, or perform a dump of state.
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

struct AllocatedBuffer 
{
	int id = 0;
	VkBuffer _buffer;
	VmaAllocation _allocation;
};

struct AllocatedImage 
{
	VkImage _image;
	VmaAllocation _allocation;
};

struct UploadContext 
{
	VkFence _uploadFence;
	VkCommandPool _commandPool;	
};

struct SwapChainSupportDetails 
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};