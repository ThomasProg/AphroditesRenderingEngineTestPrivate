#pragma once

#include "World.hpp"
#include "Window.hpp"
#include "Materials/Materials.hpp"
// #include <VulkanSubsystem.hpp>
// #include <RenderSystem.hpp>
// #include <Mesh.hpp>

#include <vk_engine.h>
#include "demoWorlds/RenderEngineDemo.hpp"


class VulkanDemo : public World
{
    // ARE::RenderSystem renderSystem;
    // Mesh mesh;

    VulkanContext* context = new VulkanContext();
    RenderEngineDemo* engine = new RenderEngineDemo();

public:
    void loadShaderModules()
    {
        const char* shaderPaths[] = 
        {
            "shaders/default_lit.frag.spv",
            "shaders/textured_lit.frag.spv",
            "shaders/testShader.frag.spv",
            "shaders/tri_mesh_ssbo.vert.spv",
            "shaders/edgeDetect.comp.spv"
        };
        
        for (const char* path : shaderPaths)
        {
            engine->addShaderModule(path);
        }
    }

    VulkanDemo(Window* w)  
    {
        engine->context = context;
        engine->resourceManager.memoryManager = context->memoryManager;

        context->init(w);
        engine->init(w);

        engine->resourceManager.addMaterial(DefaultMaterial::getRawName(), std::make_unique<DefaultMaterial>());
        engine->resourceManager.addMaterial(TexturedMaterial::getRawName(), std::make_unique<TexturedMaterial>());
        engine->resourceManager.addMaterial(TestMaterial::getRawName(), std::make_unique<TestMaterial>());

        engine->init_descriptors();

        loadShaderModules();

        engine->createFullSwapchain();

        loadResources();

        loadScene();


        // engine->passQueue.init(*context->memoryManager, VK_NULL_HANDLE, VK_NULL_HANDLE);
    }
    virtual void update() override
    {
        // static int b = 0;
        // if (b < 3)
        engine->draw();
        // b++;	
    }   
    virtual ~VulkanDemo() override
    {
        vkDeviceWaitIdle(context->memoryManager->_device);
        engine->resourceManager.clear();

        engine->cleanup();

        delete engine;

        context->cleanup();	

        delete context;
    } 

    // Load an image R8G8B8A8_SRGB / Color
    GPUTexture loadTexture(const char* str);
    void addMesh(CPUMesh& mesh, const std::string& key);

    void load_images();
    void loadResources();
    void loadScene();
};