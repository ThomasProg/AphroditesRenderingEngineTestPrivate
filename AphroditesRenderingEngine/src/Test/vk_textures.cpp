#include "Test/vk_texture.hpp"
#include <string>
#include <exception>

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
