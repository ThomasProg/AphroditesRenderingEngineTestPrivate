#include "Passes/DrawObjectsPass.hpp"

#include "vk_initializers.h"
#include "MemoryManager.hpp"

#include "Test/vk_engine.h"

void DrawObjectsPass::set(const In& in)
{
    this->in = in;
}

void DrawObjectsPass::init_descriptors(MemoryManager& memoryManager)
{
	//create a descriptor pool that will hold 10 uniform buffers
	{
		std::vector<VkDescriptorPoolSize> sizes =
		{
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = 0;
		pool_info.maxSets = 10;
		pool_info.poolSizeCount = (uint32_t)sizes.size();
		pool_info.pPoolSizes = sizes.data();
		
		vkCreateDescriptorPool(memoryManager._device, &pool_info, nullptr, &_descriptorPool);	
	}
	
	// Create global descriptor set layout
	{
		VkDescriptorSetLayoutBinding cameraBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);
		VkDescriptorSetLayoutBinding sceneBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
		
		VkDescriptorSetLayoutBinding bindings[] = { cameraBind,sceneBind };

		VkDescriptorSetLayoutCreateInfo setinfo = {};
		setinfo.bindingCount = 2;
		setinfo.flags = 0;
		setinfo.pNext = nullptr;
		setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setinfo.pBindings = bindings;

		vkCreateDescriptorSetLayout(memoryManager._device, &setinfo, nullptr, &_globalSetLayout);
	}

	// Create object description set layout
	{
		VkDescriptorSetLayoutBinding objectBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

		VkDescriptorSetLayoutCreateInfo set2info = {};
		set2info.bindingCount = 1;
		set2info.flags = 0;
		set2info.pNext = nullptr;
		set2info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		set2info.pBindings = &objectBind;

		vkCreateDescriptorSetLayout(memoryManager._device, &set2info, nullptr, &_objectSetLayout);
	}

	for (auto& matPair : resourceManager._materials)
	{
		matPair.second->createUniforms(memoryManager, _descriptorPool);
	}

	const size_t sceneParamBufferSize = FRAME_OVERLAP_X * memoryManager.pad_uniform_buffer_size(sizeof(GPUSceneData));

	_sceneParameterBuffer = memoryManager.create_buffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	
	for (int i = 0; i < FRAME_OVERLAP_X; i++)
	{
		_frames[i].cameraBuffer = memoryManager.create_buffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		const int MAX_OBJECTS = 10000;
		_frames[i].objectBuffer = memoryManager.create_buffer(sizeof(GPUObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		// Allocate global descriptor set
		{
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.pNext = nullptr;
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = _descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &_globalSetLayout;

			vkAllocateDescriptorSets(memoryManager._device, &allocInfo, &_frames[i].globalDescriptor);
		}

		// Allocate object descriptor set
		{
			VkDescriptorSetAllocateInfo objectSetAlloc = {};
			objectSetAlloc.pNext = nullptr;
			objectSetAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			objectSetAlloc.descriptorPool = _descriptorPool;
			objectSetAlloc.descriptorSetCount = 1;
			objectSetAlloc.pSetLayouts = &_objectSetLayout;

			vkAllocateDescriptorSets(memoryManager._device, &objectSetAlloc, &_frames[i].objectDescriptor);
		}

		VkDescriptorBufferInfo cameraInfo;
		cameraInfo.buffer = _frames[i].cameraBuffer._buffer;
		cameraInfo.offset = 0;
		cameraInfo.range = sizeof(GPUCameraData);

		VkDescriptorBufferInfo sceneInfo;
		sceneInfo.buffer = _sceneParameterBuffer._buffer;
		sceneInfo.offset = 0;
		sceneInfo.range = sizeof(GPUSceneData);

		VkDescriptorBufferInfo objectBufferInfo;
		objectBufferInfo.buffer = _frames[i].objectBuffer._buffer;
		objectBufferInfo.offset = 0;
		objectBufferInfo.range = sizeof(GPUObjectData) * MAX_OBJECTS;


		VkWriteDescriptorSet cameraWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _frames[i].globalDescriptor,&cameraInfo,0);
		
		VkWriteDescriptorSet sceneWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, _frames[i].globalDescriptor, &sceneInfo, 1);

		VkWriteDescriptorSet objectWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, _frames[i].objectDescriptor, &objectBufferInfo, 0);

		VkWriteDescriptorSet setWrites[] = { cameraWrite,sceneWrite,objectWrite };

		vkUpdateDescriptorSets(memoryManager._device, 3, setWrites, 0, nullptr);
	}
}


void DrawObjectsPass::setup(MemoryManager& memoryManager) 
{
    renderPass = memoryManager.init_default_renderpass(in.colorImageFormat /* VK_FORMAT_B8G8R8A8_UNORM*/, in.depthImageFormat, VK_IMAGE_LAYOUT_GENERAL);

	// Create Swapchain's depth buffer
    memoryManager.createAllocatedDepthImage2D(out.depthImage, in.depthImageFormat, in.outputImageWidth, in.outputImageHeight);

    // build a image-view for the depth image to use for rendering
    VkImageViewCreateInfo dDepthview_info = vkinit::imageview_create_info(in.depthImageFormat, out.depthImage._image, VK_IMAGE_ASPECT_DEPTH_BIT);
    VK_CHECK(vkCreateImageView(memoryManager._device, &dDepthview_info, nullptr, &out.depthImageView));

    VkFormatProperties formatProperties;

    VkFormat usedFormat = in.colorImageFormat;// VK_FORMAT_B8G8R8A8_UNORM; /*swapChainData._colorImageFormat*/

    // Get device properties for the requested texture format
    vkGetPhysicalDeviceFormatProperties(memoryManager._chosenGPU, usedFormat, &formatProperties);
    // Check if requested image format supports image storage operations
    assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);

    // src
    {
        memoryManager.createAllocatedImage2D(out.colorImage, in.colorImageFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, in.outputImageWidth, in.outputImageHeight);
        VkImageViewCreateInfo dColorview_info = vkinit::imageview_create_info(usedFormat, out.colorImage._image, VK_IMAGE_ASPECT_COLOR_BIT);
        VK_CHECK(vkCreateImageView(memoryManager._device, &dColorview_info, nullptr, &out.colorImageView));
    }

    out.sceneColorFBO = initTempFrameBuffer(memoryManager, out.colorImageView, out.depthImageView);

}

VkFramebuffer DrawObjectsPass::initTempFrameBuffer(class MemoryManager& memoryManager, VkImageView colorImageView, VkImageView depthImageView)
{
	//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	VkFramebufferCreateInfo fb_info = vkinit::framebuffer_create_info(renderPass, {in.outputImageWidth, in.outputImageHeight});

    VkImageView attachments[2];
    attachments[0] = colorImageView;
    attachments[1] = depthImageView;

    fb_info.pAttachments = attachments;
    fb_info.attachmentCount = 2;

    VkFramebuffer fbo;
    VK_CHECK(vkCreateFramebuffer(memoryManager._device, &fb_info, nullptr, &fbo));
    return fbo;
}

void DrawObjectsPass::draw_objects(MemoryManager& memoryManager, VkCommandBuffer cmd, RenderObject* first, int count)
{
	// FrameData& frameData = in.engine->get_current_frame();
	int frameID = in.engine->get_current_frame_id();
	FrameDescriptors& frameDesc = _frames[frameID];

	// ResourceManager& rm = in.engine->resourceManager;
	ResourceManager& rm = resourceManager;


	//make a model view matrix for rendering the object
	//camera view
	glm::vec3 camPos = { 0.f,-6.f,-10.f };

	glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
	//camera projection
	glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
	projection[1][1] *= -1;	

	GPUCameraData camData;
	camData.proj = projection;
	camData.view = view;
	camData.viewproj = projection * view;

	void* data;
	vmaMapMemory(memoryManager._allocator, frameDesc.cameraBuffer._allocation, &data);

	memcpy(data, &camData, sizeof(GPUCameraData));

	vmaUnmapMemory(memoryManager._allocator, frameDesc.cameraBuffer._allocation);

	for (auto& matPair : rm._materials)
	{
		assert(matPair.second.get() != nullptr);
		matPair.second->updateUniforms(memoryManager._allocator, frameID);
	}

	float framed = (glfwGetTime() / 120.f);

	in._sceneParameters.ambientColor = { sin(framed),0,cos(framed), 1};

	char* sceneData;
	vmaMapMemory(memoryManager._allocator, _sceneParameterBuffer._allocation , (void**)&sceneData);

	sceneData += memoryManager.pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameID;

	memcpy(sceneData, &in._sceneParameters, sizeof(GPUSceneData));

	vmaUnmapMemory(memoryManager._allocator, _sceneParameterBuffer._allocation);


	void* objectData;
	vmaMapMemory(memoryManager._allocator, frameDesc.objectBuffer._allocation, &objectData);
	
	GPUObjectData* objectSSBO = (GPUObjectData*)objectData;
	
	for (int i = 0; i < count; i++)
	{
		RenderObject& object = first[i];
		objectSSBO[i].modelMatrix = object.transformMatrix;
	}
	
	vmaUnmapMemory(memoryManager._allocator, frameDesc.objectBuffer._allocation);

	GPUMesh* lastMesh = nullptr;
	EngineGraphicsMaterial* lastMaterial = nullptr;
	
	for (int i = 0; i < count; i++)
	{
		RenderObject& object = first[i];

		//only bind the pipeline if it doesnt match with the already bound one
		if (object.material != lastMaterial) 
		{
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);

			object.material->bindUniforms(cmd, frameDesc.globalDescriptor, frameDesc.objectDescriptor, frameID, memoryManager._gpuProperties);

			lastMaterial = object.material;
		}

		glm::mat4 model = object.transformMatrix;
		//final render matrix, that we are calculating on the cpu
		glm::mat4 mesh_matrix = model;

		MeshPushConstants constants;
		constants.render_matrix = mesh_matrix;

		//upload the mesh to the gpu via pushconstants
		vkCmdPushConstants(cmd, object.material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

		//only bind the mesh if its a different one from last bind
		if (object.mesh != lastMesh) 
		{
			//bind the mesh vertex buffer with offset 0
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->_vertexBuffer._buffer, &offset);
			lastMesh = object.mesh;
		}
		//we can now draw
		vkCmdDraw(cmd, (uint32_t) object.mesh->size,  1,  0, i);
	}
}

void DrawObjectsPass::update(MemoryManager& memoryManager, VkCommandBuffer& cmd)
{
    // in.engine->recordColorRenderPass(cmd, renderPass, out.sceneColorFBO);

	//make a clear-color from frame number. This will flash with a 120 frame period.
	VkClearValue clearValue;
	float flash = 1.f;//abs(sin(_frameNumber / 120.f));
	clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };

	//clear depth at 1
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	//start the main renderpass. 
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(renderPass, {in.outputImageWidth, in.outputImageHeight}, out.sceneColorFBO);

	//connect clear values
	rpInfo.clearValueCount = 2;

	VkClearValue clearValues[] = { clearValue, depthClear };

	rpInfo.pClearValues = &clearValues[0];

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	// in.engine->draw_objects(cmd, _renderables.data(), (int) _renderables.size());	
	draw_objects(memoryManager, cmd, _renderables.data(), (int) _renderables.size());	

	//finalize the render pass
	vkCmdEndRenderPass(cmd);
}



void DrawObjectsPass::destroy(class MemoryManager& memoryManager)
{
	// Destroy Swapchain
	{
		vkDestroyFramebuffer(memoryManager._device, out.sceneColorFBO, nullptr);

		vkDestroyImageView(memoryManager._device, out.colorImageView, nullptr);
		// vmaDestroyImage(memoryManager._allocator, out.colorImage._image, out.colorImage._allocation); 
		memoryManager.destroyAllocatedImage(out.colorImage);
		
		
		vkDestroyImageView(memoryManager._device, out.depthImageView, nullptr);
		// vmaDestroyImage(memoryManager._allocator, out.depthImage._image, out.depthImage._allocation);
		memoryManager.destroyAllocatedImage(out.depthImage);

		vkDestroyRenderPass(memoryManager._device, renderPass, nullptr);

		// init_pipelines
		{
			for (auto& mat : resourceManager._materials)
			{
				mat.second->destroyPipeline(memoryManager);
			}
		}
	}
}

void DrawObjectsPass::firstSetup(MemoryManager& memoryManager)
{
	//init_descriptors(memoryManager);
}

void DrawObjectsPass::lastDestroy(MemoryManager& memoryManager)
{
	// Destroy
	// init_descriptors
	{

		// Destroy global and object descriptor buffers
		for (int i = 0; i < FRAME_OVERLAP_X; i++)
		{
			memoryManager.destroyBuffer(_frames[i].cameraBuffer);
			memoryManager.destroyBuffer(_frames[i].objectBuffer);
		}

		memoryManager.destroyBuffer(_sceneParameterBuffer);


		for (auto& mat : resourceManager._materials)
		{
			mat.second->destroyUniforms(memoryManager);
		}

		// Destroy global and object descriptor layouts
		vkDestroyDescriptorSetLayout(memoryManager._device, _objectSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(memoryManager._device, _globalSetLayout, nullptr);

		// Destroy descriptor pool
		vkDestroyDescriptorPool(memoryManager._device, _descriptorPool, nullptr);
	}
}

#include <fstream>

static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}



#include "Vk/ShaderModule.hpp"
bool DrawObjectsPass::load_shader_module(VkDevice _device, const char* filePath, VkShaderModule* outShaderModule)
{
	auto shaderCode = readFile(filePath);
	vk::ShaderModule fShaderModule (_device, shaderCode);
	
	*outShaderModule = fShaderModule.shaderModule;
	return true;
}

void DrawObjectsPass::addShaderModule(MemoryManager& memoryManager, const std::string& path)
{
	VkShaderModule meshVertShader;
	if (load_shader_module(memoryManager._device, path.c_str(), &meshVertShader))
	{
		resourceManager.shaderModules.emplace(path, meshVertShader);
	}
	else 
	{
		std::cout << "Error when building the mesh vertex shader module" << std::endl;
	}
}

void DrawObjectsPass::init_pipelines(MemoryManager& memoryManager, VkRenderPass renderPass)
{
	// std::unordered_map<std::string, VkShaderModule> shaderModules = loadShaders(_device);
	
	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
	PipelineBuilder pipelineBuilder = EngineGraphicsMaterial::getPipelineBuilder(in.outputImageWidth, in.outputImageHeight);

	for (auto& matPair : resourceManager._materials)
	{
		assert(matPair.second.get() != nullptr);

		EngineGraphicsMaterial& mat = *matPair.second;
		mat.makePipelineLayout(memoryManager._device, _globalSetLayout, _objectSetLayout);
		mat.makePipeline(memoryManager._device, renderPass, pipelineBuilder, resourceManager.shaderModules);
	}
}