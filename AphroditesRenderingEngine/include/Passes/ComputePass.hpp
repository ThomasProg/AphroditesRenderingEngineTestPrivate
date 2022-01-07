#pragma once

#include "Passes/Pass.hpp"

class ComputePass : public Pass
{
public:
    // ComputeMaterial mat;

public:
    virtual void setup(class MemoryManager& memoryManager);
    virtual void update(MemoryManager& memoryManager, VkCommandBuffer& cmd);

};