#include "demoWorlds/VulkanDemo.hpp"

#include "vk_engine.h"
#include <vk_initializers.h>

void VulkanDemo::addMesh(CPUMesh& mesh, const std::string& key)
{
	GPUMesh gpuMesh = context->memoryManager->uploadMesh(mesh);
	engine->resourceManager._meshes[key] = gpuMesh;
}

GPUTexture VulkanDemo::loadTexture(const char* str)
{
	CPUTexture cpuTexture;
	cpuTexture.loadFromFile("assets/lost_empire-RGBA.png");

    AllocatedImage gpuTextureAlloc = context->memoryManager->uploadTexture(cpuTexture);
	cpuTexture.destroy();

    GPUTexture gpuTexture = context->memoryManager->makeGPUTexture
    (
        gpuTextureAlloc, 
        vkinit::imageview_create_info(VK_FORMAT_R8G8B8A8_SRGB, gpuTextureAlloc._image, VK_IMAGE_ASPECT_COLOR_BIT)
    );

    return gpuTexture;
}

void VulkanDemo::load_images()
{
	engine->resourceManager._loadedTextures["empire_diffuse"] = loadTexture("assets/lost_empire-RGBA.png");
}

void VulkanDemo::loadResources()
{
    // Load Resources
	std::cout << "images" << std::endl;	
	load_images();

	std::cout << "mesh" << std::endl;

    // Add Triangles
    {
        CPUMesh triMesh{};
        //make the array 3 vertices long
        triMesh._vertices.resize(3);

        //vertex positions
        triMesh._vertices[0].position = { 1.f,1.f, 0.0f };
        triMesh._vertices[1].position = { -1.f,1.f, 0.0f };
        triMesh._vertices[2].position = { 0.f,-1.f, 0.0f };

        //vertex colors, all green
        triMesh._vertices[0].color = { 1.f,0.f, 0.0f }; 
        triMesh._vertices[1].color = { 0.f,1.f, 0.0f }; 
        triMesh._vertices[2].color = { 0.f,0.f, 1.0f }; 
        //we dont care about the vertex normals


        addMesh(triMesh, "triangle");
    }

    // Add Monkey
    {
        //load the monkey
        CPUMesh monkeyMesh{};
        monkeyMesh.load_from_obj("assets/monkey_smooth.obj");

        addMesh(monkeyMesh, "monkey");
    }

    // Add Empire
    {
        CPUMesh lostEmpire{};
        lostEmpire.load_from_obj("assets/lost_empire.obj");

        addMesh(lostEmpire, "empire");
    }
}

void VulkanDemo::loadScene()
{
	RenderObject monkey;
	monkey.mesh = engine->resourceManager.get_mesh("monkey");
	monkey.material = engine->resourceManager.get_material("defaultmesh");
	monkey.transformMatrix = glm::mat4{ 1.0f };
	engine->addRenderObject(std::move(monkey));

	RenderObject map;
	map.mesh = engine->resourceManager.get_mesh("empire");
	map.material = engine->resourceManager.get_material(TexturedMaterial::getRawName());
	map.transformMatrix = glm::translate(glm::vec3{ 5,-10,0 }); //glm::mat4{ 1.0f };
    engine->addRenderObject(std::move(map));

	for (int x = -20; x <= 20; x++) 
    {
		for (int y = -20; y <= 20; y++) 
        {

			RenderObject tri;
			tri.mesh = engine->resourceManager.get_mesh("triangle");
			tri.material = engine->resourceManager.get_material("defaultmesh");
			glm::mat4 translation = glm::translate(glm::mat4{ 1.0 }, glm::vec3(x, 0, y));
			glm::mat4 scale = glm::scale(glm::mat4{ 1.0 }, glm::vec3(0.2, 0.2, 0.2));
			tri.transformMatrix = translation * scale;

            engine->addRenderObject(std::move(tri));
		}
	}

	for (auto& matPair : engine->resourceManager._materials)
	{
		matPair.second->initUniforms(*engine, context->memoryManager->_device, engine->_descriptorPool);
	}
}