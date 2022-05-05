#pragma once

#include <vk_types.h>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

struct VertexInputDescription 
{
	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;

	VkPipelineVertexInputStateCreateFlags flags = 0;

	VkPipelineVertexInputStateCreateInfo toVertexInputInfo();
};


struct Vertex 
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv;
	static VertexInputDescription get_vertex_description();
}; 

struct CPUMesh 
{
	std::vector<Vertex> _vertices;

	bool load_from_obj(const char* filename);
};

struct GPUMesh 
{
	AllocatedBuffer _vertexBuffer;
	size_t size;

	// void destroy(VmaAllocator _allocator)
	// {
	// 	vmaDestroyBuffer(_allocator, _vertexBuffer._buffer, _vertexBuffer._allocation);
	// }
};