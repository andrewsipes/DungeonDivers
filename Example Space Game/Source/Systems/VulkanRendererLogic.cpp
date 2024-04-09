//#include "VulkanRendererLogic.h"
//#include "../Components/Identification.h"
//#include "../Components/Visuals.h"
//#include "../Components/Physics.h"
//
//using namespace ESG; // Example Space Game
//
//bool ESG::VulkanRendererLogic::Init(	std::shared_ptr<flecs::world> _game, 
//								std::weak_ptr<const GameConfig> _gameConfig,
//								GW::GRAPHICS::GVulkanSurface _vulkan,
//								GW::SYSTEM::GWindow _window)
//{
//	// save a handle to the ECS & game settings
//	game = _game;
//	gameConfig = _gameConfig;
//	vulkan = _vulkan;
//	window = _window;
//	// Setup all vulkan resources
//	if (LoadShaders() == false) 
//		return false;
//	if (LoadUniforms() == false)
//		return false;
//	if (LoadGeometry() == false)
//		return false;
//	if (SetupPipeline() == false)
//		return false;
//	// Setup drawing engine
//	if (SetupDrawcalls() == false)
//		return false;
//	// GVulkanSurface will inform us when to release any allocated resources
//	shutdown.Create(vulkan, [&]() {
//		if (+shutdown.Find(GW::GRAPHICS::GVulkanSurface::Events::RELEASE_RESOURCES, true)) {
//			FreeVulkanResources(); // unlike D3D we must be careful about destroy timing
//		}
//	}); 
//	return true;
//}
//
//bool ESG::VulkanRendererLogic::Activate(bool runSystem)
//{
//	if (startDraw.is_alive() &&
//		updateDraw.is_alive() &&
//		completeDraw.is_alive()) {
//		if (runSystem) {
//			startDraw.enable();
//			updateDraw.enable();
//			completeDraw.enable();
//		}
//		else {
//			startDraw.disable();
//			updateDraw.disable();
//			completeDraw.disable();
//		}
//		return true;
//	}
//	return false;
//}
//
//bool ESG::VulkanRendererLogic::Shutdown()
//{
//	startDraw.destruct();
//	updateDraw.destruct();
//	completeDraw.destruct();
//	return true; // vulkan resource shutdown handled via GEvent in Init()
//}
//
//std::string ESG::VulkanRendererLogic::ShaderAsString(const char* shaderFilePath)
//{
//	std::string output;
//	unsigned int stringLength = 0;
//	GW::SYSTEM::GFile file; file.Create();
//	file.GetFileSize(shaderFilePath, stringLength);
//	if (stringLength && +file.OpenBinaryRead(shaderFilePath)) {
//		output.resize(stringLength);
//		file.Read(&output[0], stringLength);
//	}
//	else
//		std::cout << "ERROR: Shader Source File \"" << shaderFilePath << "\" Not Found!" << std::endl;
//	return output;
//}
//
//bool ESG::VulkanRendererLogic::LoadShaders()
//{
//	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
//	std::string vertexShaderSource = (*readCfg).at("Shaders").at("vertex").as<std::string>();
//	std::string pixelShaderSource = (*readCfg).at("Shaders").at("pixel").as<std::string>();
//	
//	if (vertexShaderSource.empty() || pixelShaderSource.empty())
//		return false;
//
//	vertexShaderSource = ShaderAsString(vertexShaderSource.c_str());
//	pixelShaderSource = ShaderAsString(pixelShaderSource.c_str());
//
//	if (vertexShaderSource.empty() || pixelShaderSource.empty())
//		return false;
//	
//	VkDevice device = nullptr;
//	vulkan.GetDevice((void**)&device);
//	shaderc_compiler_t compiler = shaderc_compiler_initialize();
//	shaderc_compile_options_t options = shaderc_compile_options_initialize();
//	shaderc_compile_options_set_source_language(options, shaderc_source_language_hlsl);
//	shaderc_compile_options_set_invert_y(options, true);
//#ifndef NDEBUG
//	shaderc_compile_options_set_generate_debug_info(options);
//#endif
//	// Create Vertex Shader
//	shaderc_compilation_result_t result = shaderc_compile_into_spv( // compile
//		compiler, vertexShaderSource.c_str(), vertexShaderSource.length(),
//		shaderc_vertex_shader, "main.vert", "main", options);
//	if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
//		std::cout << "Vertex Shader Errors: " << shaderc_result_get_error_message(result) << std::endl;
//	GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
//		(char*)shaderc_result_get_bytes(result), &vertexShader);
//	shaderc_result_release(result); // done
//	// Create Pixel Shader
//	result = shaderc_compile_into_spv( // compile
//		compiler, pixelShaderSource.c_str(), pixelShaderSource.length(),
//		shaderc_fragment_shader, "main.frag", "main", options);
//	if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
//		std::cout << "Pixel Shader Errors: " << shaderc_result_get_error_message(result) << std::endl;
//	GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
//		(char*)shaderc_result_get_bytes(result), &pixelShader);
//	shaderc_result_release(result); // done
//	// Free runtime shader compiler resources
//	shaderc_compile_options_release(options);
//	shaderc_compiler_release(compiler);
//	return true;
//}
//
//bool ESG::VulkanRendererLogic::LoadUniforms()
//{
//	VkDevice device = nullptr;
//	VkPhysicalDevice physicalDevice = nullptr;
//	vulkan.GetDevice((void**)&device);
//	vulkan.GetPhysicalDevice((void**)&physicalDevice);
//
//	unsigned max_frames = 0;
//	// to avoid per-frame resource sharing issues we give each "in-flight" frame its own buffer
//	vulkan.GetSwapchainImageCount(max_frames);
//	uniformHandle.resize(max_frames);
//	uniformData.resize(max_frames);
//	for (int i = 0; i < max_frames; ++i) {
//	
//		if (VK_SUCCESS != GvkHelper::create_buffer(physicalDevice, device,
//			sizeof(INSTANCE_UNIFORMS), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//			&uniformHandle[i], &uniformData[i]))
//			return false;
//			
//		if (VK_SUCCESS != GvkHelper::write_to_buffer( device, uniformData[i], 
//			&instanceData, sizeof(INSTANCE_UNIFORMS)))
//			return false; 
//	}
//	// uniform buffers created
//	return true;
//}
//
//bool ESG::VulkanRendererLogic::LoadGeometry()
//{
//	VkDevice device = nullptr;
//	VkPhysicalDevice physicalDevice = nullptr;
//	vulkan.GetDevice((void**)&device);
//	vulkan.GetPhysicalDevice((void**)&physicalDevice);
//	
//	float verts[] = {
//		-0.5f, -0.5f,
//		0, 0.5f,
//		0, -0.25f,
//		0.5f, -0.5f
//	};
//	// Transfer triangle data to the vertex buffer. (staging buffer would be prefered here)
//	if (VK_SUCCESS == GvkHelper::create_buffer(physicalDevice, device, sizeof(verts),
//		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
//		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexHandle, &vertexData)) {
//		if (VK_SUCCESS == GvkHelper::write_to_buffer(device, vertexData, verts, sizeof(verts))) {
//			return true;
//		}
//	}
//	return false;
//}
//
//bool ESG::VulkanRendererLogic::SetupPipeline()
//{
//	VkDevice device = nullptr;
//	vulkan.GetDevice((void**)&device);
//	VkRenderPass renderPass;
//	vulkan.GetRenderPass((void**)&renderPass);
//	VkPipelineShaderStageCreateInfo stage_create_info[2] = {};
//	// Create Stage Info for Vertex Shader
//	stage_create_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//	stage_create_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
//	stage_create_info[0].module = vertexShader;
//	stage_create_info[0].pName = "main";
//	// Create Stage Info for Fragment Shader
//	stage_create_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//	stage_create_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//	stage_create_info[1].module = pixelShader;
//	stage_create_info[1].pName = "main";
//	// Assembly State
//	VkPipelineInputAssemblyStateCreateInfo assembly_create_info = {};
//	assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//	assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
//	assembly_create_info.primitiveRestartEnable = false;
//	// Vertex Input State
//	VkVertexInputBindingDescription vertex_binding_description = {};
//	vertex_binding_description.binding = 0;
//	vertex_binding_description.stride = sizeof(float) * 2;
//	vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//	VkVertexInputAttributeDescription vertex_attribute_description[1] = {
//		{ 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 } //uv, normal, etc....
//	};
//	VkPipelineVertexInputStateCreateInfo input_vertex_info = {};
//	input_vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//	input_vertex_info.vertexBindingDescriptionCount = 1;
//	input_vertex_info.pVertexBindingDescriptions = &vertex_binding_description;
//	input_vertex_info.vertexAttributeDescriptionCount = 1;
//	input_vertex_info.pVertexAttributeDescriptions = vertex_attribute_description;
//	// Viewport State (we still need to set this up even though we will overwrite the values)
//	VkViewport viewport = {
//        0, 0, static_cast<float>(1920), static_cast<float>(1080), 0, 1
//    };
//    VkRect2D scissor = { {0, 0}, {1920, 1080} }; // we will overwrite this in Draw
//	VkPipelineViewportStateCreateInfo viewport_create_info = {};
//	viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//	viewport_create_info.viewportCount = 1;
//	viewport_create_info.pViewports = &viewport;
//	viewport_create_info.scissorCount = 1;
//	viewport_create_info.pScissors = &scissor;
//	// Rasterizer State
//	VkPipelineRasterizationStateCreateInfo rasterization_create_info = {};
//	rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//	rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;
//	rasterization_create_info.polygonMode = VK_POLYGON_MODE_FILL;
//	rasterization_create_info.lineWidth = 1.0f;
//	rasterization_create_info.cullMode = VK_CULL_MODE_NONE; // disable culling
//	rasterization_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
//	rasterization_create_info.depthClampEnable = VK_FALSE;
//	rasterization_create_info.depthBiasEnable = VK_FALSE;
//	rasterization_create_info.depthBiasClamp = 0.0f;
//	rasterization_create_info.depthBiasConstantFactor = 0.0f;
//	rasterization_create_info.depthBiasSlopeFactor = 0.0f;
//	// Multisampling State
//	VkPipelineMultisampleStateCreateInfo multisample_create_info = {};
//	multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//	multisample_create_info.sampleShadingEnable = VK_FALSE;
//	multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
//	multisample_create_info.minSampleShading = 1.0f;
//	multisample_create_info.pSampleMask = VK_NULL_HANDLE;
//	multisample_create_info.alphaToCoverageEnable = VK_FALSE;
//	multisample_create_info.alphaToOneEnable = VK_FALSE;
//	// Depth-Stencil State
//	VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
//	depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//	depth_stencil_create_info.depthTestEnable = VK_TRUE;
//	depth_stencil_create_info.depthWriteEnable = VK_TRUE;
//	depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
//	depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
//	depth_stencil_create_info.minDepthBounds = 0.0f;
//	depth_stencil_create_info.maxDepthBounds = 1.0f;
//	depth_stencil_create_info.stencilTestEnable = VK_FALSE;
//	// Color Blending Attachment & State
//	VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
//	color_blend_attachment_state.colorWriteMask = 0xF;
//	color_blend_attachment_state.blendEnable = VK_FALSE;
//	color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
//	color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
//	color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
//	color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
//	color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
//	color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
//	VkPipelineColorBlendStateCreateInfo color_blend_create_info = {};
//	color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//	color_blend_create_info.logicOpEnable = VK_FALSE;
//	color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
//	color_blend_create_info.attachmentCount = 1;
//	color_blend_create_info.pAttachments = &color_blend_attachment_state;
//	color_blend_create_info.blendConstants[0] = 0.0f;
//	color_blend_create_info.blendConstants[1] = 0.0f;
//	color_blend_create_info.blendConstants[2] = 0.0f;
//	color_blend_create_info.blendConstants[3] = 0.0f;
//	// Dynamic State 
//	VkDynamicState dynamic_state[2] = { 
//		// By setting these we do not need to re-create the pipeline on Resize
//		VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
//	};
//	VkPipelineDynamicStateCreateInfo dynamic_create_info = {};
//	dynamic_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//	dynamic_create_info.dynamicStateCount = 2;
//	dynamic_create_info.pDynamicStates = dynamic_state;
//	// Descriptor Setup
//	VkDescriptorSetLayoutBinding descriptor_layout_binding = {};
//	descriptor_layout_binding.binding = 0;
//	descriptor_layout_binding.descriptorCount = 1;
//	descriptor_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//	// In this scenario we have the same descriptorSetLayout for both shaders...
//	// However, many times you would want seperate layouts for each since they tend to have different needs 
//	descriptor_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
//	descriptor_layout_binding.pImmutableSamplers = nullptr;
//	VkDescriptorSetLayoutCreateInfo descriptor_create_info = {};
//	descriptor_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//	descriptor_create_info.flags = 0; 
//	descriptor_create_info.bindingCount = 1;
//	descriptor_create_info.pBindings = &descriptor_layout_binding;
//	descriptor_create_info.pNext = nullptr;
//	// Descriptor layout
//	vkCreateDescriptorSetLayout(device, &descriptor_create_info, nullptr, &descriptorLayout);
//	// Create a descriptor pool!
//	unsigned max_frames = 0;
//	vulkan.GetSwapchainImageCount(max_frames);
//	VkDescriptorPoolCreateInfo descriptorpool_create_info = {};
//	descriptorpool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//	VkDescriptorPoolSize descriptorpool_size = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, max_frames };
//	descriptorpool_create_info.poolSizeCount = 1;
//	descriptorpool_create_info.pPoolSizes = &descriptorpool_size;
//	descriptorpool_create_info.maxSets = max_frames;
//	descriptorpool_create_info.flags = 0;
//	descriptorpool_create_info.pNext = nullptr;
//	vkCreateDescriptorPool(device, &descriptorpool_create_info, nullptr, &descriptorPool);
//	// Create a descriptorSet for each uniform buffer!
//	VkDescriptorSetAllocateInfo descriptorset_allocate_info = {};
//	descriptorset_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//	descriptorset_allocate_info.descriptorSetCount = 1;
//	descriptorset_allocate_info.pSetLayouts = &descriptorLayout;
//	descriptorset_allocate_info.descriptorPool = descriptorPool;
//	descriptorset_allocate_info.pNext = nullptr;
//	descriptorSet.resize(max_frames);
//	for (int i = 0; i < max_frames; ++i) {
//		vkAllocateDescriptorSets(device, &descriptorset_allocate_info, &descriptorSet[i]);
//	}
//	// link descriptor sets to uniform buffers (one for each bufferimage)
//	VkWriteDescriptorSet write_descriptorset = {};
//	write_descriptorset.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//	write_descriptorset.descriptorCount = 1;
//	write_descriptorset.dstArrayElement = 0;
//	write_descriptorset.dstBinding = 0;
//	write_descriptorset.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//	for (int i = 0; i < max_frames; ++i) {
//		write_descriptorset.dstSet = descriptorSet[i];
//		VkDescriptorBufferInfo dbinfo = { uniformHandle[i], 0, VK_WHOLE_SIZE };
//		write_descriptorset.pBufferInfo = &dbinfo;
//		vkUpdateDescriptorSets(device, 1, &write_descriptorset, 0, nullptr);
//	}
//	// Descriptor pipeline layout
//	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
//	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//	pipeline_layout_create_info.setLayoutCount = 1;
//	pipeline_layout_create_info.pSetLayouts = &descriptorLayout;
//	pipeline_layout_create_info.pushConstantRangeCount = 0;
//	pipeline_layout_create_info.pPushConstantRanges = nullptr;
//	vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &pipelineLayout);
//	// Pipeline State... (FINALLY) 
//	VkGraphicsPipelineCreateInfo pipeline_create_info = {};
//	pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//	pipeline_create_info.stageCount = 2;
//	pipeline_create_info.pStages = stage_create_info;
//	pipeline_create_info.pInputAssemblyState = &assembly_create_info;
//	pipeline_create_info.pVertexInputState = &input_vertex_info;
//	pipeline_create_info.pViewportState = &viewport_create_info;
//	pipeline_create_info.pRasterizationState = &rasterization_create_info;
//	pipeline_create_info.pMultisampleState = &multisample_create_info;
//	pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;
//	pipeline_create_info.pColorBlendState = &color_blend_create_info;
//	pipeline_create_info.pDynamicState = &dynamic_create_info;
//	pipeline_create_info.layout = pipelineLayout;
//	pipeline_create_info.renderPass = renderPass;
//	pipeline_create_info.subpass = 0;
//	pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
//	if (VK_SUCCESS != vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
//		&pipeline_create_info, nullptr, &pipeline))
//		return false; // something went wrong
//
//	return true;
//}
//
//bool ESG::VulkanRendererLogic::SetupDrawcalls() // I SCREWED THIS UP MAKES SO MUCH SENSE NOW
//{
//	// create a unique entity for the renderer (just a Tag)
//	// this only exists to ensure we can create systems that will run only once per frame. 
//	struct RenderingSystem {}; // local definition so we control iteration counts
//	game->entity("Rendering System").add<RenderingSystem>();
//	// an instanced renderer is complex and needs to run additional system code once per frame
//	// to do this I create 3 systems:
//	// A pre-update system, that runs only once (using our Tag above)
//	// An update system that iterates over all renderable components (may run multiple times)
//	// A post-update system that also runs only once rendering all collected data
//
//	// only happens once per frame
//	startDraw = game->system<RenderingSystem>().kind(flecs::PreUpdate)
//		.each([this](flecs::entity e, RenderingSystem& s) {
//		// reset the draw counter only once per frame
//		draw_counter = 0; 
//	});
//	// may run multiple times per frame, will run after startDraw
//	updateDraw = game->system<Position, Orientation, Material>().kind(flecs::OnUpdate)
//		.each([this](flecs::entity e, Position& p, Orientation& o, Material& m) {
//		// copy all data to our instancing array
//		int i = draw_counter; 
//		instanceData.instance_transforms[i] = GW::MATH::GIdentityMatrixF;
//		instanceData.instance_transforms[i].row4.x = p.value.x;
//		instanceData.instance_transforms[i].row4.y = p.value.y;
//		// transfer 2D orientation to 4x4
//		instanceData.instance_transforms[i].row1.x = o.value.row1.x;
//		instanceData.instance_transforms[i].row1.y = o.value.row1.y;
//		instanceData.instance_transforms[i].row2.x = o.value.row2.x;
//		instanceData.instance_transforms[i].row2.y = o.value.row2.y;
//		// set color
//		instanceData.instance_colors[i].x = m.diffuse.value.x;
//		instanceData.instance_colors[i].y = m.diffuse.value.y;
//		instanceData.instance_colors[i].z = m.diffuse.value.z;
//		instanceData.instance_colors[i].w = 1; // opaque
//		// increment the shared draw counter but don't go over (branchless) 
//		int v = static_cast<int>(Instance_Max) - static_cast<int>(draw_counter + 2);
//		// if v < 0 then 0, else 1, https://graphics.stanford.edu/~seander/bithacks.html
//		int sign = 1 ^ ((unsigned int)v >> (sizeof(int) * CHAR_BIT - 1)); 
//		draw_counter += sign;
//	});
//	// runs once per frame after updateDraw
//	completeDraw = game->system<RenderingSystem>().kind(flecs::PostUpdate)
//		.each([this](flecs::entity e, RenderingSystem& s) {
//		// run the rendering code just once!
//		// Copy data to this frame's buffer
//		VkDevice device = nullptr;
//		vulkan.GetDevice((void**)&device);
//		unsigned int activeBuffer;
//		vulkan.GetSwapchainCurrentImage(activeBuffer);
//		GvkHelper::write_to_buffer(device, 
//			uniformData[activeBuffer], &instanceData, sizeof(INSTANCE_UNIFORMS));
//		// grab the current Vulkan commandBuffer
//		unsigned int currentBuffer;
//		vulkan.GetSwapchainCurrentImage(currentBuffer);
//		VkCommandBuffer commandBuffer;
//		vulkan.GetCommandBuffer(currentBuffer, (void**)&commandBuffer);
//		// what is the current client area dimensions?
//		unsigned int width, height;
//		window.GetClientWidth(width);
//		window.GetClientHeight(height);
//		// setup the pipeline's dynamic settings
//		VkViewport viewport = {
//			0, 0, static_cast<float>(width), static_cast<float>(height), 0, 1
//		};
//		VkRect2D scissor = { {0, 0}, {width, height} };
//		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
//		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
//		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
//		// Set the descriptorSet that contains the uniform buffer allocated for this framebuffer 
//		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
//			pipelineLayout, 0, 1, &descriptorSet[currentBuffer], 0, nullptr);
//		// now we can draw
//		VkDeviceSize offsets[] = { 0 };
//		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexHandle, offsets);
//		vkCmdDraw(commandBuffer, 4, draw_counter, 0, 0); // draw'em all!
//	});
//	// NOTE: I went with multi-system approach for the ease of passing lambdas with "this"
//	// There is a built-in solution for this problem referred to as a "custom runner":
//	// https://github.com/SanderMertens/flecs/blob/master/examples/cpp/systems/custom_runner/src/main.cpp
//	// The negative is that it requires the use of a C callback which is less flexibe vs the lambda
//	// you could embed what you need in the ecs and use a lookup to get it but I think that is less clean
//	
//	// all drawing operations have been setup
//	return true;
//}
//
//bool ESG::VulkanRendererLogic::FreeVulkanResources()
//{
//	VkDevice device = nullptr;
//	vulkan.GetDevice((void**)&device);
//	// wait till everything has completed
//	vkDeviceWaitIdle(device);
//	// free the uniform buffer and its handle
//	for (int i = 0; i < uniformData.size(); ++i) {
//		vkDestroyBuffer(device, uniformHandle[i], nullptr);
//		vkFreeMemory(device, uniformData[i], nullptr);
//	}
//	uniformData.clear(); 
//	uniformHandle.clear();
//	// don't need the descriptors anymore
//	vkDestroyDescriptorSetLayout(device, descriptorLayout, nullptr);
//	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
//	// Release allocated buffers, shaders & pipeline
//	vkDestroyBuffer(device, vertexHandle, nullptr);
//	vkFreeMemory(device, vertexData, nullptr);
//	vkDestroyShaderModule(device, vertexShader, nullptr);
//	vkDestroyShaderModule(device, pixelShader, nullptr);
//	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
//	vkDestroyPipeline(device, pipeline, nullptr);
//	return true;
//}
