#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Window.hpp"
#include "Vk/Surface.hpp"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.hpp"

// Class doing the presentation = displays to the viewport
class Presenter
{
public:
    vk::Surface surface;
    PhysicalDevice physicalDevice;
    LogicalDevice device;

    void init(VkInstance instance, Window& window)
    {
        surface.init(instance, window);
        physicalDevice.physicalDevice = pickPhysicalDevice(instance, surface.surface);
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface.surface);
        device.init(indices, physicalDevice);
    }

    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

    static bool isDeviceSuitable(QueueFamilyIndices indices, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

    static bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    static VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
};

#include "SwapChain.hpp"

inline bool Presenter::isDeviceSuitable(QueueFamilyIndices indices, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = SwapChain::querySwapChainSupport(surface, physicalDevice);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

inline QueueFamilyIndices Presenter::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) 
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) 
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
        {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) 
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) 
        {
            break;
        }

        i++;
    }

    return indices;
}

inline bool Presenter::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}


inline VkPhysicalDevice Presenter::pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) 
{
    VkPhysicalDevice physicalDevice;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) 
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) 
    {
        QueueFamilyIndices indices = findQueueFamilies(device, surface);
        if (isDeviceSuitable(indices, device, surface)) 
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) 
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    return physicalDevice;
}
