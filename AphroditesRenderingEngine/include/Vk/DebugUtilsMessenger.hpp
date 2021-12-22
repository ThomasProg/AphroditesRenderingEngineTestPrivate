#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vk
{
    class DebugUtilsMessengerEXT
    {
    public:
        VkDebugUtilsMessengerEXT debugMessenger;

    public:

        inline operator VkDebugUtilsMessengerEXT()
        {
            return debugMessenger;
        }


        static inline VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
        {
            auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
            if (func != nullptr) 
            {
                return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
            } else 
            {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }
        }

        static inline void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) 
        {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr) 
            {
                func(instance, debugMessenger, pAllocator);
            }
        }

        void setupDebugMessenger(VkInstance instance) 
        {
            if (!enableValidationLayers) 
                return;

            VkDebugUtilsMessengerCreateInfoEXT createInfo;
            vk::Instance::populateDebugMessengerCreateInfo(createInfo);

            if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to set up debug messenger!");
            }
        }

        static VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance) 
        {
            if (!enableValidationLayers) 
                return VkDebugUtilsMessengerEXT();

            VkDebugUtilsMessengerCreateInfoEXT createInfo;
            vk::Instance::populateDebugMessengerCreateInfo(createInfo);

            VkDebugUtilsMessengerEXT debugMessenger;
            if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to set up debug messenger!");
            }
            return debugMessenger;
        }

        // void setupDebugMessenger(VkInstance instance) {
        //     if (!enableValidationLayers) return;

        //     VkDebugUtilsMessengerCreateInfoEXT createInfo;
        //     populateDebugMessengerCreateInfo(createInfo);

        //     if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        //         throw std::runtime_error("failed to set up debug messenger!");
        //     }
        // }

        
        // static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        //     createInfo = {};
        //     createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        //     createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        //     createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        //     createInfo.pfnUserCallback = debugCallback;
        // }

        // static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        //     std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        //     return VK_FALSE;
        // }
    };
}