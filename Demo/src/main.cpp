#include <thread>
#include <iostream>

// #include "World.hpp"
#include "World.hpp"
#include "WorldManager.hpp"
#include "demoWorlds/VulkanDemo.hpp"
#include "WindowContext.hpp"
#include "Window.hpp"

int main()
{
    // runMain();

    try {
        WindowContext windowContext;
        std::unique_ptr<Window> window (windowContext.MakeWindow());

        WorldManager wManager;
        wManager.setCurrentWorld(wManager.addWorld<VulkanDemo>(window.get()));

        while (!glfwWindowShouldClose(window->window)) 
        {
            glfwPollEvents();

            wManager.getCurrent()->update();

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}