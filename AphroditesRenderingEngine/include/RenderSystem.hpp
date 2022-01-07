// #pragma once

// #include "Mesh.hpp"
// #include "MeshHandle.hpp"

// namespace ARE
// {

// class RenderSystem
// {
// public:
//     std::vector<MeshHandle> handles;
//     HelloTriangleApplication app;

//     RenderSystem(Window* w) : app(w) 
//     {

//     }

//     void init()
//     {
//         app.initVulkan();
//     }

//     void destroy()
//     {
//         app.cleanup();

//         for (MeshHandle& handle : handles)
//         {
//             handle.indexBuffer.destroy(app.presenter.device);
//             handle.vertexBuffer.destroy(app.presenter.device);
//         }

//         app.cleanupEnd();
//     }

//     void drawFrame()
//     {
//         app.drawFrame();
//     }

//     void addMesh(Mesh& mesh)
//     {
//         MeshHandle handle;
//         handle.nbIndices = mesh.indices.size();
//         handle.nbVertices = mesh.vertices.size();
//         handles.emplace_back(std::move(handle));

//         std::cout << " === Vertex Buffer === " << std::endl;
//         handles.back().vertexBuffer = app.createVertexBuffer(mesh, app.presenter, app.commandPool, app.presenter.device.graphicsQueue);
//         std::cout << " === Index Buffer === " << std::endl;
//         handles.back().indexBuffer = app.createIndexBuffer(mesh);
//         std::cout << " === init end === " << std::endl;

//         {
//             int width = 0, height = 0;
//             glfwGetFramebufferSize(app.window->window, &width, &height);
//             while (width == 0 || height == 0)
//             {
//                 glfwGetFramebufferSize(app.window->window, &width, &height);
//                 glfwWaitEvents();
//             }

//             vkDeviceWaitIdle(app.presenter.device);

//             app.cleanupSwapChain();

//             app.createSwapChain();
//             app.swapChain.createImageViews();
//             app.createRenderPass();
//             app.createGraphicsPipeline();
//             app.createDepthResources();
//             app.createFramebuffers();
//             app.createUniformBuffers();
//             app.createDescriptorPool();
//             app.createDescriptorSets();
//             app.createCommandBuffers(handles);

//             app.handles = handles;

//             app.imagesInFlight.resize(app.swapChain.swapChainImages.size(), VK_NULL_HANDLE);
//         }

//         std::cout << "Remade swapchain" << std::endl;
//     }
// };

// }