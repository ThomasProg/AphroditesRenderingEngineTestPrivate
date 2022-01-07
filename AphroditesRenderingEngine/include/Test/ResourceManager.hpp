#pragma once

#include <memory>
#include <unordered_map>

#include "Material.hpp"
#include "Test/vk_mesh.h"
#include "Test/vk_texture.hpp"

// GPU Resources
struct ResourceManager
{
    MemoryManager* memoryManager = nullptr;

	std::unordered_map<std::string, std::unique_ptr<GraphicsMaterial>> _materials;
	std::unordered_map<std::string, GPUMesh> _meshes;
	std::unordered_map<std::string, GPUTexture> _loadedTextures;
	std::unordered_map<std::string, VkShaderModule> shaderModules;
	
	GraphicsMaterial* addMaterial(const std::string& name, std::unique_ptr<GraphicsMaterial>&& mat);

	// //create material and add it to the map
	// Material* create_material(const std::string& name);

	//returns nullptr if it cant be found
	GraphicsMaterial* get_material(const std::string& name);

	//returns nullptr if it cant be found
	GPUMesh* get_mesh(const std::string& name);

	void clear();
};
