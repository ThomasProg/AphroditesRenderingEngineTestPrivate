#include "Test/ResourceManager.hpp"

// include class MemoryManager
#include "Test/vk_engine.h"

void ResourceManager::clear()
{
	for (auto& mesh : _meshes)
	{
		mesh.second.destroy(memoryManager->_allocator);
	}

	_meshes.clear();

	for (auto& texture : _loadedTextures)
	{
		vmaDestroyImage(memoryManager->_allocator, texture.second.image._image, texture.second.image._allocation);
		vkDestroyImageView(memoryManager->_device, texture.second.imageView, nullptr);	
	}

	_loadedTextures.clear();

	for (auto& shaderModule : shaderModules)
	{
		vkDestroyShaderModule(memoryManager->_device, shaderModule.second, nullptr);
	}

	shaderModules.clear();
}

GraphicsMaterial* ResourceManager::addMaterial(const std::string& name, std::unique_ptr<GraphicsMaterial>&& mat)
{
	GraphicsMaterial* p = mat.get();
	_materials[name] = std::move(mat);
	return p;
}

GraphicsMaterial* ResourceManager::get_material(const std::string& name)
{
	//search for the object, and return nullpointer if not found
	auto it = _materials.find(name);
	if (it == _materials.end()) {
		return nullptr;
	}
	else {
		return (*it).second.get();
	}
}

GPUMesh* ResourceManager::get_mesh(const std::string& name)
{
	auto it = _meshes.find(name);
	if (it == _meshes.end()) {
		return nullptr;
	}
	else {
		return &(*it).second;
	}
}