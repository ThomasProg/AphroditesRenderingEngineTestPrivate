#include "Texture.hpp"
#include <stdexcept>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture::Texture(char const* path) : pixels(stbi_load(path, &width, &height, &nbChannels, STBI_rgb_alpha)) // images will be stored in rgba
{

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
}

Texture::Texture(int width, int height, int nbChannels) : width(width), height(height), pixels((stbi_uc *) stbi__malloc(width * height)), nbChannels(nbChannels)
{

}

Texture::~Texture()
{
    stbi_image_free(pixels);
}

