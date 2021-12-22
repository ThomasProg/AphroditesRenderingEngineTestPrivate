#pragma once

namespace vk
{

class ShaderModule
{
public:
    VkShaderModule shaderModule;

public:
    ShaderModule(VkDevice device, const std::vector<char>& code)
    {
        shaderModule = createShaderModule(device, code);
    }
    
    static VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) 
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    void destroy(VkDevice device)
    {
        vkDestroyShaderModule(device, shaderModule, nullptr);
    }

    operator VkShaderModule()
    {
        return shaderModule;    
    }
};

}