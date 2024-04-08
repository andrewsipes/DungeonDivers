namespace DRAW {
	
	//*** TAGS ***//
	
	//*** COMPONENTS ***//
	struct VulkanPiplineInitialization {
		uint32_t stride;
		int windowWidth;
		int windowHeight;
	};

	struct VulkanPipeline {
		// settings for the pipeline will be based on what is attached to the entity on_update
		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		
		// *** IMPORTANT *** these need to be moved to their own component, but I just want to prove it works first
 
		// non-geometry resources need descriptor sets/pools/layouts to be connected to shaders
		VkDescriptorSetLayout descriptorLayout = nullptr; // describes order of connection to shaders
		VkDescriptorPool descriptorPool = nullptr; // used to allocate descriptorSets (required)
		// DescriptorSets are not synchronized with drawing so we will need one for each set of resources
		// Generally we need one per framebuffer per-pipeline (assuming different shader resources)
		std::vector<VkDescriptorSet> descriptorSet; // not-plural since we need multiple for one resource
	};
	
	//*** SYSTEMS ***//
	
	inline void Construct_VulkanPipeline(entt::registry& registry, entt::entity entity) {
		auto& component = registry.get<VulkanPipeline>(entity);
		// nothing to do here, we only build/re-build the pipeline when the component is updated 
	}

	inline void Update_VulkanPipeline(entt::registry& registry, entt::entity entity) {
		auto& vkPipeline = registry.get<VulkanPipeline>(entity);
		if (!registry.all_of<VulkanPiplineInitialization>(entity))
		{
			std::cout << "VulkanPipeline: Missing Vulkan Pipeline initialization data!" << std::endl;
			abort();
			return;
		}

		if (!registry.all_of<VulkanVertexShader, VulkanFragmentShader>(entity))
		{
			std::cout << "VulkanPipeline: Missing Shader components" << std::endl;
			abort();
			return;
		}

		auto& vkSurface = registry.get<GW::GRAPHICS::GVulkanSurface>(entity);
		auto& vkPipelineInitialization = registry.get<VulkanPiplineInitialization>(entity);

		VkDevice device = nullptr;
		vkSurface.GetDevice((void**)&device);

		VkRenderPass renderPass;
		vkSurface.GetRenderPass((void**)&renderPass);

#pragma region Set the Shaders
		VkPipelineShaderStageCreateInfo stage_create_info[2] = {};
		// Create Stage Info for Vertex Shader
		stage_create_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_create_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		stage_create_info[0].module = registry.get<DRAW::VulkanVertexShader>(entity).vertexShader;
		stage_create_info[0].pName = "main";

		// Create Stage Info for Fragment Shader
		stage_create_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_create_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		stage_create_info[1].module = registry.get<VulkanFragmentShader>(entity).fragmentShader;
		stage_create_info[1].pName = "main";

#pragma endregion

#pragma region Vertex Descriptions
		VkPipelineInputAssemblyStateCreateInfo assembly_create_info = {};
		assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		assembly_create_info.primitiveRestartEnable = false;

		VkVertexInputBindingDescription vertex_binding_description = {};
		vertex_binding_description.binding = 0;
		vertex_binding_description.stride = vkPipelineInitialization.stride;
		vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription vertex_attribute_description[3];
		vertex_attribute_description[0].binding = 0;
		vertex_attribute_description[0].location = 0;
		vertex_attribute_description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertex_attribute_description[0].offset = 0;

		vertex_attribute_description[1].binding = 0;
		vertex_attribute_description[1].location = 1;
		vertex_attribute_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertex_attribute_description[1].offset = 12;

		vertex_attribute_description[2].binding = 0;
		vertex_attribute_description[2].location = 2;
		vertex_attribute_description[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertex_attribute_description[2].offset = 24;


		VkPipelineVertexInputStateCreateInfo input_vertex_info = {};
		input_vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		input_vertex_info.vertexBindingDescriptionCount = 1;
		input_vertex_info.pVertexBindingDescriptions = &vertex_binding_description;
		input_vertex_info.vertexAttributeDescriptionCount = 3;
		input_vertex_info.pVertexAttributeDescriptions = vertex_attribute_description;
#pragma endregion

#pragma region Viewport Setup
		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = static_cast<float>(vkPipelineInitialization.windowWidth);
		viewport.height = static_cast<float>(vkPipelineInitialization.windowHeight);
		viewport.minDepth = 0;
		viewport.maxDepth = 1;

		VkRect2D scissor = {};
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = vkPipelineInitialization.windowWidth;
		scissor.extent.height = vkPipelineInitialization.windowHeight;

		VkPipelineViewportStateCreateInfo viewport_create_info = {};
		viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_create_info.viewportCount = 1;
		viewport_create_info.pViewports = &viewport;
		viewport_create_info.scissorCount = 1;
		viewport_create_info.pScissors = &scissor;
#pragma endregion

#pragma region Rasterization Setup
		VkPipelineRasterizationStateCreateInfo rasterization_create_info = {};
		rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;
#if ENABLE_WIREFRAME
		rasterization_create_info.polygonMode = VK_POLYGON_MODE_LINE;
#else
		rasterization_create_info.polygonMode = VK_POLYGON_MODE_FILL;
#endif
		rasterization_create_info.lineWidth = 1.0f;
		rasterization_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterization_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterization_create_info.depthClampEnable = VK_FALSE;
		rasterization_create_info.depthBiasEnable = VK_FALSE;
		rasterization_create_info.depthBiasClamp = 0.0f;
		rasterization_create_info.depthBiasConstantFactor = 0.0f;
		rasterization_create_info.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisample_create_info = {};
		multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_create_info.sampleShadingEnable = VK_FALSE;
		multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_create_info.minSampleShading = 1.0f;
		multisample_create_info.pSampleMask = VK_NULL_HANDLE;
		multisample_create_info.alphaToCoverageEnable = VK_FALSE;
		multisample_create_info.alphaToOneEnable = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
		depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_create_info.depthTestEnable = VK_TRUE;
		depth_stencil_create_info.depthWriteEnable = VK_TRUE;
		depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_create_info.minDepthBounds = 0.0f;
		depth_stencil_create_info.maxDepthBounds = 1.0f;
		depth_stencil_create_info.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
		color_blend_attachment_state.colorWriteMask = 0xF;
		color_blend_attachment_state.blendEnable = VK_FALSE;
		color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
		color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
		color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
		color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo color_blend_create_info = {};
		color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_create_info.logicOpEnable = VK_FALSE;
		color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
		color_blend_create_info.attachmentCount = 1;
		color_blend_create_info.pAttachments = &color_blend_attachment_state;
		color_blend_create_info.blendConstants[0] = 0.0f;
		color_blend_create_info.blendConstants[1] = 0.0f;
		color_blend_create_info.blendConstants[2] = 0.0f;
		color_blend_create_info.blendConstants[3] = 0.0f;
#pragma endregion

#pragma region Descriptor Sets
		//InitializeDescriptorSets();
#pragma endregion

		// Dynamic State 
		VkDynamicState dynamic_states[2] =
		{
			// By setting these we do not need to re-create the pipeline on Resize
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamic_create_info = {};
		dynamic_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_create_info.dynamicStateCount = 2;
		dynamic_create_info.pDynamicStates = dynamic_states;

		// Descriptor pipeline layout
		// TODO: Buffer setups
		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.setLayoutCount = 0;
		pipeline_layout_create_info.pSetLayouts = VK_NULL_HANDLE;
		pipeline_layout_create_info.pushConstantRangeCount = 0;
		pipeline_layout_create_info.pPushConstantRanges = nullptr;

		vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &vkPipeline.pipelineLayout);

		// Pipeline State... (FINALLY) 
		VkGraphicsPipelineCreateInfo pipeline_create_info = {};
		pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_create_info.stageCount = 2;
		pipeline_create_info.pStages = stage_create_info;
		pipeline_create_info.pInputAssemblyState = &assembly_create_info;
		pipeline_create_info.pVertexInputState = &input_vertex_info;
		pipeline_create_info.pViewportState = &viewport_create_info;
		pipeline_create_info.pRasterizationState = &rasterization_create_info;
		pipeline_create_info.pMultisampleState = &multisample_create_info;
		pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;
		pipeline_create_info.pColorBlendState = &color_blend_create_info;
		pipeline_create_info.pDynamicState = &dynamic_create_info;
		pipeline_create_info.layout = vkPipeline.pipelineLayout;
		pipeline_create_info.renderPass = renderPass;
		pipeline_create_info.subpass = 0;
		pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;

		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &vkPipeline.pipeline);

		// remove the initialization since it's no longer needed
		registry.remove<VulkanPiplineInitialization>(entity);
	}

	inline void Destroy_VulkanPipeline(entt::registry& registry, entt::entity entity) {
		auto& vkPipeline = registry.get<VulkanPipeline>(entity);

		auto& vkSurface = registry.get<GW::GRAPHICS::GVulkanSurface>(entity);

		VkDevice device = nullptr;
		vkSurface.GetDevice((void**)&device);

		vkDestroyPipelineLayout(device, vkPipeline.pipelineLayout, nullptr);
		vkDestroyPipeline(device, vkPipeline.pipeline, nullptr);
	}
	
	// MACRO to connect the EnTT VulkanPipeline Logic(s) listed above to the EnTT registry
	CONNECT_COMPONENT_LOGIC() {
		// Connect the logic to the registry for your new component
		registry.on_construct<VulkanPipeline>().connect<Construct_VulkanPipeline>();
		registry.on_update<VulkanPipeline>().connect<Update_VulkanPipeline>();
		registry.on_destroy<VulkanPipeline>().connect<Destroy_VulkanPipeline>();
	}
	
} // namespace DRAW