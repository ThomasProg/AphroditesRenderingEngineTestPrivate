# Aphrodite's Rendering Engine

To Build : 
```
cmake -B Build/Debug
cmake --build Build/Debug --config Debug --target AphroditesRenderingEngine Demo

cmake --build Build/Debug --config Debug --target Janus
```

.lib : 
```
Build\Debug\AphroditesRenderingEngine\Debug
```

TODO : 
- Split :
    - Vulkan initialization
    - Swapchain creation
    - Resources initialization

- Materials : 
    - Make "polymorphic" materials (inheritance to initialize / destroy materials)
    - Possess multiple textures