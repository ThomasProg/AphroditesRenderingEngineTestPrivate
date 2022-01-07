#include "Passes/ComputePass.hpp"


void ComputePass::setup(class MemoryManager& memoryManager) {}


void ComputePass::update(MemoryManager& memoryManager, VkCommandBuffer& cmd)
{
    // vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, mat.pipeline);
    // mat.bindUniforms(cmd, memoryManager->_gpuProperties);
    // vkCmdDispatch(cmd, width / 16, height / 16, 1);
}
