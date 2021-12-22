#pragma once

#include "World.hpp"
#include "Window.hpp"
// #include <VulkanSubsystem.hpp>
// #include <RenderSystem.hpp>
// #include <Mesh.hpp>

#include <vk_engine.h>



class VulkanDemo : public World
{
    // ARE::RenderSystem renderSystem;
    // Mesh mesh;

	VulkanEngine engine;

public:
    VulkanDemo(Window* w) //: renderSystem(w) 
    {
        engine.init(w);

        loadResources();

        loadScene();
    }
    virtual void update() override
    {
        engine.run();	
    }   
    virtual ~VulkanDemo() override
    {
        engine.cleanup();	
    } 

    void loadResources();
    void loadScene();
};