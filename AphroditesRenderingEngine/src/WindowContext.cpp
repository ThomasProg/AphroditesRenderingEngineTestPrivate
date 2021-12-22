#include "WindowContext.hpp"
#include "Window.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

WindowContext::WindowContext()
{
    glfwInit();
}

WindowContext::~WindowContext()
{
    glfwTerminate();
}

Window* WindowContext::MakeWindow()
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    Window* window = new Window(800, 600, "Vulkan");
    return window;
} 
