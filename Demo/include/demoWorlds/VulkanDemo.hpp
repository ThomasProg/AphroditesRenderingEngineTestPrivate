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
            engine->drawObjectsPass.addShaderModule(*context->memoryManager, path);
        }
    }

    VulkanDemo(Window* w)  
    {
        engine->context = context;
        engine->drawObjectsPass.resourceManager.memoryManager = context->memoryManager;

        context->init(w);
        engine->init(w);

        engine->drawObjectsPass.resourceManager.addMaterial(DefaultMaterial::getRawName(), std::make_unique<DefaultMaterial>());
        engine->drawObjectsPass.resourceManager.addMaterial(TexturedMaterial::getRawName(), std::make_unique<TexturedMaterial>());
        engine->drawObjectsPass.resourceManager.addMaterial(TestMaterial::getRawName(), std::make_unique<TestMaterial>());

        engine->drawObjectsPass.init_descriptors(*context->memoryManager);

        loadShaderModules();

        engine->createFullSwapchain();

        // //build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
        // PipelineBuilder pipelineBuilder = EngineGraphicsMaterial::getPipelineBuilder(*engine->_window);

        // for (auto& matPair : engine->resourceManager._materials)
        // {
        //     assert(matPair.second.get() != nullptr);

        //     EngineGraphicsMaterial& mat = *matPair.second;
        //     mat.makePipelineLayout(context->memoryManager->_device, engine->_globalSetLayout, engine->_objectSetLayout);
        //     mat.makePipeline(context->memoryManager->_device, engine->_renderPass, pipelineBuilder, engine->resourceManager.shaderModules);
        // }

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
        engine->drawObjectsPass.resourceManager.clear();

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