#include "Materials/Materials.hpp"

#include "vk_engine.h"

#include <vk_initializers.h>

void DefaultMaterial::makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules)
{
	GraphicsMaterial::makePipeline(_device, _renderPass, pipelineBuilder, shaderModules, "shaders/tri_mesh_ssbo.vert.spv", "shaders/default_lit.frag.spv");
}

void TestMaterial::makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules)
{
	GraphicsMaterial::makePipeline(_device, _renderPass, pipelineBuilder, shaderModules, "shaders/tri_mesh_ssbo.vert.spv", "shaders/testShader.frag.spv");
	// Material::makePipeline(_device, _renderPass, pipelineBuilder, shaderModules, "shaders/tri_mesh_ssbo.vert.spv", "shaders/testShader.frag.spv");
}



void TexturedMaterial::makePipelineLayout(VkDevice _device, VkDescriptorSetLayout _globalSetLayout, VkDescriptorSetLayout _objectSetLayout)
{
	VkDescriptorSetLayout texturedSetLayouts[] = { _globalSetLayout, _objectSetLayout, shaderLayout /*texturedShaderUniforms.shaderLayout*/ };
	GraphicsMaterial::makePipelineLayout(_device, texturedSetLayouts, 3);
}

void TexturedMaterial::makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules)
{
	GraphicsMaterial::makePipeline(_device, _renderPass, pipelineBuilder, shaderModules, "shaders/tri_mesh_ssbo.vert.spv", "shaders/textured_lit.frag.spv");
}

void TexturedMaterial::updateUniforms(VmaAllocator _allocator, int frameID)
{
    struct TestStruct
    {
        float multValue[4];
    };

    TestStruct testData;
    for (int i = 0; i < 4; i++)
    {
        testData.multValue[i] = fmod((float)glfwGetTime(), 1.f);
    }

    void* data;
    
    AllocatedBuffer& buffer = uniformBuffers[frameID]; 
    vmaMapMemory(_allocator, buffer._allocation, &data);

    memcpy(data, &testData, sizeof(TestStruct));

    vmaUnmapMemory(_allocator, buffer._allocation);
}

void TexturedMaterial::createUniforms(VulkanEngine& context, VkDevice device, VkDescriptorPool _descriptorPool) 
{
	// Create Descriptor Set Layout
	{
		VkDescriptorSetLayoutBinding textureBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
		VkDescriptorSetLayoutBinding testBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);

		constexpr size_t nbBindings = 2;
		VkDescriptorSetLayoutBinding  t[nbBindings] = {textureBind, testBind};

		VkDescriptorSetLayoutCreateInfo set3info = {};
		set3info.bindingCount = nbBindings;
		set3info.flags = 0;
		set3info.pNext = nullptr;
		set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		set3info.pBindings = t;

		vkCreateDescriptorSetLayout(device, &set3info, nullptr, &shaderLayout);
	}

	// Allocate Descriptor Set
	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.pNext = nullptr;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = _descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &shaderLayout /*engine.texturedShaderUniforms.shaderLayout*/;

		if (matDescriptors == VK_NULL_HANDLE)
			vkAllocateDescriptorSets(device, &allocInfo, &matDescriptors);
	}

	{
		std::cout << "VK_NULL_HANDLE : " << VK_NULL_HANDLE << std::endl; 

		VkSamplerCreateInfo samplerInfo = vkinit::sampler_create_info(VK_FILTER_NEAREST);

		vkCreateSampler(device, &samplerInfo, nullptr, &blockySampler);
	}
}

void TexturedMaterial::initUniforms(class VulkanEngine& engine, VkDevice device, VkDescriptorPool _descriptorPool) 
{
	VkDescriptorImageInfo imageBufferInfo;
	imageBufferInfo.sampler = blockySampler;
	imageBufferInfo.imageView = engine.resourceManager._loadedTextures["empire_diffuse"].imageView;
	imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet texture1 = vkinit::write_descriptor_image(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, matDescriptors, &imageBufferInfo, 0);


	std::vector<VkWriteDescriptorSet> sets;
	sets.resize(1 + FRAME_OVERLAP);
	sets[FRAME_OVERLAP] = texture1;

	uniformBuffers.resize(FRAME_OVERLAP);
	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		// engine._frames[i].testBuffer = engine.create_buffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		uniformBuffers[i] = engine.context->memoryManager->create_buffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		struct TestStruct
		{
			float multValue;
		};

		VkDescriptorBufferInfo testInfo;
		testInfo.buffer = uniformBuffers[i]._buffer; // engine.get_current_frame().testBuffer._buffer;
		testInfo.offset = 0;
		testInfo.range = sizeof(TestStruct);
		VkWriteDescriptorSet testWriteDescriptor = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, matDescriptors,&testInfo,1);

		sets[i] = testWriteDescriptor;
	}

	// VkWriteDescriptorSet t[2] = {texture1, testWriteDescriptor};

	vkUpdateDescriptorSets(device, (uint32_t) sets.size(), sets.data(), 0, nullptr);
}

void TexturedMaterial::destroyUniforms(class MemoryManager& memoryManager) 
{
	for  (AllocatedBuffer& buffer : uniformBuffers)
	{
		// Test Material Destroy
		memoryManager.destroyBuffer(buffer);
	}

	vkDestroySampler(memoryManager._device, blockySampler, nullptr);
	vkDestroyDescriptorSetLayout(memoryManager._device, shaderLayout, nullptr);
}

void TexturedMaterial::bindUniforms(VkCommandBuffer cmd, VkDescriptorSet& globalDescriptor, VkDescriptorSet& objectDescriptor, int frameIndex, VkPhysicalDeviceProperties _gpuProperties)
{
    GraphicsMaterial::bindUniforms(cmd, globalDescriptor, objectDescriptor, frameIndex, _gpuProperties);

    if (matDescriptors != VK_NULL_HANDLE) 
    {
        //texture descriptor
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &matDescriptors, 0, nullptr);
    }
}






void EdgeDetectMaterial::makePipelineLayout(MemoryManager& memoryManager)
{
	// No push constants for compute shader

	//we start from  the normal mesh layout
	VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info(); // mesh_pipeline_layout_info;

	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &shaderLayout;

	VK_CHECK(vkCreatePipelineLayout(memoryManager._device, &pipeline_layout_info, nullptr, &pipelineLayout));
}

inline VkComputePipelineCreateInfo GetComputePipelineCreateInfo(VkPipelineLayout layout, VkPipelineCreateFlags flags = 0)
{
	VkComputePipelineCreateInfo computePipelineCreateInfo {};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.layout = layout;
	computePipelineCreateInfo.flags = flags;
	return computePipelineCreateInfo;
}

void EdgeDetectMaterial::makePipeline(MemoryManager& memoryManager, const std::unordered_map<std::string, VkShaderModule>& shaderModules)
{
	VkComputePipelineCreateInfo computePipelineCreateInfo = GetComputePipelineCreateInfo(pipelineLayout, 0);
	computePipelineCreateInfo.stage = vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_COMPUTE_BIT, shaderModules.at("shaders/edgeDetect.comp.spv")); // loadShader(fileName, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHECK(vkCreateComputePipelines(memoryManager._device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &pipeline));
}



void EdgeDetectMaterial::createUniforms(MemoryManager& memoryManager, VkDescriptorPool descriptorPool)
{
	// Create Descriptor Set Layout
	{
		VkDescriptorSetLayoutBinding inImageDescriptorBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0);
		VkDescriptorSetLayoutBinding outImageDescriptorBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1);

		constexpr size_t nbBindings = 2;
		VkDescriptorSetLayoutBinding  t[nbBindings] = {inImageDescriptorBind, outImageDescriptorBind};

		VkDescriptorSetLayoutCreateInfo set3info = {};
		set3info.bindingCount = nbBindings;
		set3info.flags = 0;
		set3info.pNext = nullptr;
		set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		set3info.pBindings = t;

		vkCreateDescriptorSetLayout(memoryManager._device, &set3info, nullptr, &shaderLayout);
	}

	// Allocate Descriptor Set
	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.pNext = nullptr;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &shaderLayout /*engine.texturedShaderUniforms.shaderLayout*/;

		// if (matDescriptors == VK_NULL_HANDLE)
		vkAllocateDescriptorSets(memoryManager._device, &allocInfo, &matDescriptors);
	}
}

void EdgeDetectMaterial::initUniforms(MemoryManager& memoryManager) 
{
	// VkDescriptorImageInfo inImageDescriptor;
    // inImageDescriptor.imageView;
    // inImageDescriptor.imageLayout;

	// VkDescriptorImageInfo outImageDescriptor;
    // outImageDescriptor.imageView;
    // outImageDescriptor.imageLayout;

	std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets = {
		vkinit::write_descriptor_image(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, matDescriptors, &inImageDescriptor, 0),
		vkinit::write_descriptor_image(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, matDescriptors, &outImageDescriptor, 1),
		// vks::initializers::writeDescriptorSet(compute.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &textureColorMap.descriptor),
		// vks::initializers::writeDescriptorSet(compute.descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &textureComputeTarget.descriptor)
	};
	vkUpdateDescriptorSets(memoryManager._device, (uint32_t) computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);
}

void EdgeDetectMaterial::bind(MemoryManager& memoryManager, VkCommandBuffer cmd) 
{
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &matDescriptors, 0, nullptr);
}

void EdgeDetectMaterial::destroyUniforms(class MemoryManager& memoryManager)
{
	vkDestroyDescriptorSetLayout(memoryManager._device, shaderLayout, nullptr);
	// matDescriptors = VK_NULL_HANDLE;
}
