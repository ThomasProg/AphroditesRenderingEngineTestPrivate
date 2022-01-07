#pragma once

#include <vulkan/vulkan.h>
#include <vk_types.h>


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