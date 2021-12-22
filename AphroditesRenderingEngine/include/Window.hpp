#pragma once

#include <functional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window
{
public:
    GLFWwindow* window = nullptr;
    int width;
    int height;

    Window(int width, int height, const char* title)
        : window(glfwCreateWindow(width, height, title, nullptr, nullptr))
    {
        this->width = width;
        this->height = height;
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    ~Window()
    {
        glfwDestroyWindow(window);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) 
    {       
        // auto w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

        // w->width = width;
        // w->height = height;

        // w->onResize(w);
    }

public:
    std::function<void(Window*)> onResize = [](Window*) {};

};
