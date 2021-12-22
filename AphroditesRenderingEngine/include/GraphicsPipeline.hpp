#pragma once

class GraphicsPipeline
{
public:
    static VkPipelineShaderStageCreateInfo getVertexShaderStageCreateInfo(VkShaderModule shaderModule, const char* mainName = "main")
    {
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = shaderModule;
        vertShaderStageInfo.pName = mainName;

        return vertShaderStageInfo;
    }

    static VkPipelineShaderStageCreateInfo getFragmentShaderStageCreateInfo(VkShaderModule shaderModule, const char* mainName = "main")
    {
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = shaderModule;
        fragShaderStageInfo.pName = mainName;

        return fragShaderStageInfo;
    }

    static VkViewport getViewport(float width, float height)
    {
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = width;
        viewport.height = height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        return viewport;
    }

    // void createGraphicsPipeline()
    // {
    //     VkGraphicsPipelineCreateInfo pipelineInfo{};
    //     pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    //     pipelineInfo.stageCount = 2;
    //     pipelineInfo.pStages = shaderStages;
    //     pipelineInfo.pVertexInputState = &vertexInputInfo;
    //     pipelineInfo.pInputAssemblyState = &inputAssembly;
    //     pipelineInfo.pViewportState = &viewportState;
    //     pipelineInfo.pRasterizationState = &rasterizer;
    //     pipelineInfo.pMultisampleState = &multisampling;
    //     pipelineInfo.pDepthStencilState = &depthStencil;
    //     pipelineInfo.pColorBlendState = &colorBlending;
    //     pipelineInfo.layout = pipelineLayout;
    //     pipelineInfo.renderPass = renderPass;
    //     pipelineInfo.subpass = 0;
    //     pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    //     if (vkCreateGraphicsPipelines(presenter.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) 
    //     {
    //         throw std::runtime_error("failed to create graphics pipeline!");
    //     }
    // }
};


