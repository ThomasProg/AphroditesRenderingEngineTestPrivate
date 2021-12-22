#include "demoWorlds/VulkanDemo.hpp"

#include "vk_engine.h"
#include <vk_initializers.h>

void VulkanDemo::loadResources()
{
    // Load Resources
	std::cout << "images" << std::endl;	
	engine.load_images();

	std::cout << "mesh" << std::endl;
	// engine.load_meshes();

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


        engine.addMesh(triMesh, "triangle");
    }

    // Add Monkey
    {
        //load the monkey
        CPUMesh monkeyMesh{};
        monkeyMesh.load_from_obj("assets/monkey_smooth.obj");

        engine.addMesh(monkeyMesh, "monkey");
    }

    // Add Empire
    {
        CPUMesh lostEmpire{};
        lostEmpire.load_from_obj("assets/lost_empire.obj");

        engine.addMesh(lostEmpire, "empire");
    }
}

void VulkanDemo::loadScene()
{
	RenderObject monkey;
	monkey.mesh = engine.resourceManager.get_mesh("monkey");
	monkey.material = engine.resourceManager.get_material("defaultmesh");
	monkey.transformMatrix = glm::mat4{ 1.0f };
	engine.addRenderObject(std::move(monkey));

	RenderObject map;
	map.mesh = engine.resourceManager.get_mesh("empire");
	map.material = engine.resourceManager.get_material("texturedmesh");
	map.transformMatrix = glm::translate(glm::vec3{ 5,-10,0 }); //glm::mat4{ 1.0f };
    engine.addRenderObject(std::move(map));

	for (int x = -20; x <= 20; x++) 
    {
		for (int y = -20; y <= 20; y++) 
        {

			RenderObject tri;
			tri.mesh = engine.resourceManager.get_mesh("triangle");
			tri.material = engine.resourceManager.get_material("defaultmesh");
			glm::mat4 translation = glm::translate(glm::mat4{ 1.0 }, glm::vec3(x, 0, y));
			glm::mat4 scale = glm::scale(glm::mat4{ 1.0 }, glm::vec3(0.2, 0.2, 0.2));
			tri.transformMatrix = translation * scale;

            engine.addRenderObject(std::move(tri));
		}
	}


	Material* texturedMat =	engine.resourceManager.get_material("texturedmesh");

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.pNext = nullptr;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = engine._descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &texturedMat->shaderLayout /*engine.texturedShaderUniforms.shaderLayout*/;

	if (texturedMat->matDescriptors == VK_NULL_HANDLE)
		vkAllocateDescriptorSets(engine._device, &allocInfo, &texturedMat->matDescriptors);

	std::cout << "Texture set value : " << texturedMat->matDescriptors << std::endl; 
	std::cout << "VK_NULL_HANDLE : " << VK_NULL_HANDLE << std::endl; 

	VkSamplerCreateInfo samplerInfo = vkinit::sampler_create_info(VK_FILTER_NEAREST);

	vkCreateSampler(engine._device, &samplerInfo, nullptr, &engine.blockySampler);

	VkDescriptorImageInfo imageBufferInfo;
	imageBufferInfo.sampler = engine.blockySampler;
	imageBufferInfo.imageView = engine.resourceManager._loadedTextures["empire_diffuse"].imageView;
	imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet texture1 = vkinit::write_descriptor_image(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texturedMat->matDescriptors, &imageBufferInfo, 0);


	std::vector<VkWriteDescriptorSet> sets;
	sets.resize(1 + FRAME_OVERLAP);
	sets[FRAME_OVERLAP] = texture1;

	texturedMat->uniformBuffers.resize(FRAME_OVERLAP);
	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		// engine._frames[i].testBuffer = engine.create_buffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		texturedMat->uniformBuffers[i] = engine.create_buffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		struct TestStruct
		{
			float multValue;
		};

		VkDescriptorBufferInfo testInfo;
		testInfo.buffer = texturedMat->uniformBuffers[i]._buffer; // engine.get_current_frame().testBuffer._buffer;
		testInfo.offset = 0;
		testInfo.range = sizeof(TestStruct);
		VkWriteDescriptorSet testWriteDescriptor = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, texturedMat->matDescriptors,&testInfo,1);

		sets[i] = testWriteDescriptor;
	}

	// VkWriteDescriptorSet t[2] = {texture1, testWriteDescriptor};

	vkUpdateDescriptorSets(engine._device, sets.size(), sets.data(), 0, nullptr);
}