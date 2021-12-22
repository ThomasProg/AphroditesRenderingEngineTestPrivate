#pragma once

#include <vector>
#include "BoundBuffer.hpp"
#include "UniformBuffer.hpp"

class MeshHandle
{
public:
    size_t nbVertices;
    size_t nbIndices;

    BoundBuffer vertexBuffer;
    BoundBuffer indexBuffer;

    // std::vector<UniformBuffer> uniformBuffers;
};