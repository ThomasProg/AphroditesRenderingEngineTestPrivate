#include "Material.hpp"

#include "Test/vk_engine.h"
#include <vk_initializers.h>
#include "Window.hpp"
#include "MemoryManager.hpp"

VkPipeline PipelineBuilder::build_pipeline(VkDevice device, VkRenderPass pass)
{
	//make viewport state from our stored viewport and scissor.
		//at the moment we wont support multiple viewports or scissors
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;

	viewportState.viewportCount = 1;
	viewportState.pViewports = &_viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &_scissor;

	//setup dummy color blending. We arent using transparent objects yet
	//the blending is just "no blend", but we do write to the color attachment
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext = nullptr;

	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &_colorBlendAttachment;

	//build the actual pipeline
	//we now use all of the info structs we have been writing into into this one to create the pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;

	pipelineInfo.stageCount = (uint32_t) _shaderStages.size();
	pipelineInfo.pStages = _shaderStages.data();
	pipelineInfo.pVertexInputState = &_vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &_inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &_rasterizer;
	pipelineInfo.pMultisampleState = &_multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDepthStencilState = &_depthStencil;
	pipelineInfo.layout = _pipelineLayout;
	pipelineInfo.renderPass = pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	//its easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
	VkPipeline newPipeline;
	if (vkCreateGraphicsPipelines(
		device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) {
		std::cout << "failed to create pipline\n";
		return VK_NULL_HANDLE; // failed to create graphics pipeline
	}
	else
	{
		return newPipeline;
	}
}


void GraphicsMaterial::bindUniforms(VkCommandBuffer cmd, VkDescriptorSet& globalDescriptor, VkDescriptorSet& objectDescriptor, int frameIndex, VkPhysicalDeviceProperties _gpuProperties)
{
	uint32_t uniform_offset = (uint32_t) MemoryManager::pad_uniform_buffer_size(sizeof(GPUSceneData), _gpuProperties.limits.minUniformBufferOffsetAlignment) * frameIndex;
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &globalDescriptor, 1, &uniform_offset);

	//object data descriptor
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &objectDescriptor, 0, nullptr);
}

PipelineBuilder GraphicsMaterial::getPipelineBuilder(Window& window)
{
	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
	PipelineBuilder pipelineBuilder;

	//vertex input controls how to read vertices from vertex buffers. We arent using it yet
	pipelineBuilder._vertexInputInfo = vkinit::vertex_input_state_create_info();

	//input assembly is the configuration for drawing triangle lists, strips, or individual points.
	//we are just going to draw triangle list
	pipelineBuilder._inputAssembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	//build viewport and scissor from the swapchain extents
	pipelineBuilder._viewport.x = 0.0f;
	pipelineBuilder._viewport.y = 0.0f;
	pipelineBuilder._viewport.width = (float)window.width;
	pipelineBuilder._viewport.height = (float)window.height;
	pipelineBuilder._viewport.minDepth = 0.0f;
	pipelineBuilder._viewport.maxDepth = 1.0f;

	pipelineBuilder._scissor.offset = { 0, 0 };
	pipelineBuilder._scissor.extent = {(uint32_t)window.width, (uint32_t)window.height};

	//configure the rasterizer to draw filled triangles
	pipelineBuilder._rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);

	//we dont use multisampling, so just run the default one
	pipelineBuilder._multisampling = vkinit::multisampling_state_create_info();

	//a single blend attachment with no blending and writing to RGBA
	pipelineBuilder._colorBlendAttachment = vkinit::color_blend_attachment_state();


	//default depthtesting
	pipelineBuilder._depthStencil = vkinit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	// //build the mesh pipeline
	// VertexInputDescription vertexDescription = Vertex::get_vertex_description();

	// //connect the pipeline builder vertex input info to the one we get from Vertex
	// pipelineBuilder._vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
	// pipelineBuilder._vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

	// pipelineBuilder._vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
	// pipelineBuilder._vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

	return pipelineBuilder;
}


void GraphicsMaterial::makePipelineLayout(VkDevice _device, VkDescriptorSetLayout* layouts, int nbLayouts)
{
	//we start from  the normal mesh layout
	VkPipelineLayoutCreateInfo textured_pipeline_layout_info = vkinit::pipeline_layout_create_info(); // mesh_pipeline_layout_info;

	VkPushConstantRange push_constant;
	//offset 0
	push_constant.offset = 0;
	//size of a MeshPushConstant struct
	push_constant.size = sizeof(MeshPushConstants);
	//for the vertex shader
	push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	textured_pipeline_layout_info.pPushConstantRanges = &push_constant;
	textured_pipeline_layout_info.pushConstantRangeCount = 1;

	textured_pipeline_layout_info.setLayoutCount = nbLayouts;
	textured_pipeline_layout_info.pSetLayouts = layouts;

	VK_CHECK(vkCreatePipelineLayout(_device, &textured_pipeline_layout_info, nullptr, &pipelineLayout));
}

void GraphicsMaterial::makePipelineLayout(VkDevice _device, VkDescriptorSetLayout _globalSetLayout, VkDescriptorSetLayout _objectSetLayout)
{
	VkDescriptorSetLayout layouts[] = { _globalSetLayout, _objectSetLayout };
	makePipelineLayout(_device, layouts, 2);
}

void GraphicsMaterial::makePipeline(VkDevice _device, VkRenderPass _renderPass, PipelineBuilder pipelineBuilder, const std::unordered_map<std::string, VkShaderModule>& shaderModules, const std::string& vertexShader, const std::string& fragmentShader)
{
	VertexInputDescription vertexDescription = Vertex::get_vertex_description();
	pipelineBuilder._vertexInputInfo = vertexDescription.toVertexInputInfo();

	//hook the push constants layout
	pipelineBuilder._pipelineLayout = pipelineLayout;
	
	//build the mesh triangle pipeline
	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, shaderModules.at(vertexShader)));

	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, shaderModules.at(fragmentShader)));

	pipeline = pipelineBuilder.build_pipeline(_device, _renderPass);
}


void Material::destroyPipeline(class MemoryManager& memoryManager)
{
	vkDestroyPipeline(memoryManager._device, pipeline, nullptr);
	vkDestroyPipelineLayout(memoryManager._device, pipelineLayout, nullptr);
}

