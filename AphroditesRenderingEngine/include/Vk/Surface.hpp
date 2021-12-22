#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Window.hpp"

namespace vk
{

class Surface
{
public:
    VkSurfaceKHR surface;

    void init(VkInstance instance, Window& window)
    {
        if (glfwCreateWindowSurface(instance, window.window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void destroy(VkInstance instance)
    {
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }
};

}