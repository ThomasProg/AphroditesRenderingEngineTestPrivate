@REM C:/VulkanSDK/1.2.182.0/Bin/glslc.exe shaders/shader.vert -o shaders/vert.spv
@REM C:/VulkanSDK/1.2.182.0/Bin/glslc.exe shaders/shader.frag -o shaders/frag.spv

C:/VulkanSDK/1.2.182.0/Bin/glslc.exe glslShaders/default_lit.frag -o shaders/default_lit.frag.spv
C:/VulkanSDK/1.2.182.0/Bin/glslc.exe glslShaders/textured_lit.frag -o shaders/textured_lit.frag.spv
C:/VulkanSDK/1.2.182.0/Bin/glslc.exe glslShaders/testShader.frag -o shaders/testShader.frag.spv
C:/VulkanSDK/1.2.182.0/Bin/glslc.exe glslShaders/tri_mesh_ssbo.vert -o shaders/tri_mesh_ssbo.vert.spv

C:/VulkanSDK/1.2.182.0/Bin/glslc.exe glslShaders/edgeDetect.comp -o shaders/edgeDetect.comp.spv

pause