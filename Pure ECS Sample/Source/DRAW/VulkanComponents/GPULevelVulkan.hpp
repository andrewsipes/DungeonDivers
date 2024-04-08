namespace DRAW {
	
	//*** TAGS ***//

	//*** COMPONENTS ***//
	struct GPULevelVulkan {
		GW::SYSTEM::GConcurrent async;
	};
	struct BlenderName {
		std::string name;
	};
	
	//*** SYSTEMS ***//
	
	inline void Construct_GPULevelVulkan(entt::registry& registry, entt::entity entity) {
		auto* cpu = registry.try_get<CPULevel>(entity);
		if (cpu == nullptr) {
			std::cout << "GPULevel requires a CPULevel component to be present" << std::endl;
			return;
		}
		auto& gpu = registry.get<GPULevelVulkan>(entity);

		gpu.async.Create(true);
		gpu.async.BranchSingular([&registry, entity, cpu] {
			// wait for CPU to complete loading the level data	
			cpu->async.Converge(0);

			// convert all buffers from the CPULevel to GPU buffers

			// vertex buffer
			registry.emplace<VulkanVertexBuffer>(entity);
			registry.emplace<std::vector<H2B::VERTEX>>(entity, cpu->data.levelVertices); // consider std::move
			registry.patch<VulkanVertexBuffer>(entity); // loads the CPU buffer into the GPU buffer

			// index buffer
			registry.emplace<VulkanIndexBuffer>(entity);
			registry.emplace<std::vector<unsigned int>>(entity, cpu->data.levelIndices); // consider std::move
			registry.patch<VulkanIndexBuffer>(entity); // loads the CPU buffer into the GPU buffer

			// I think this should actually be connected to the VulkanPipeline, not the GPULevel
			// storage buffer (used for caching instance matrices and materials during traversal)
			registry.emplace<VulkanStorageBuffer<GPU_INSTANCE>>(entity, 
				VulkanStorageBuffer<GPU_INSTANCE>{ cpu->data.levelTransforms.size() });

			// generate drawable entities for each Blender object's sub-mesh in the CPULevel
			// each entity should have: H2B::BATCH, GPU_INSTANCE, BlenderName, (+ VulkanPipeline handle later) 
			for (auto& object : cpu->data.blenderObjects) {
				auto& model = cpu->data.levelModels[object.modelIndex];
				for (int m = 0; m < model.meshCount; m++) {
					auto& meshInfo = cpu->data.levelMeshes[model.meshStart + m];
					auto drawable = registry.create();
					registry.emplace<H2B::BATCH>(drawable, meshInfo.drawInfo);
					registry.emplace<GPU_INSTANCE>(drawable, GPU_INSTANCE{
						cpu->data.levelTransforms[object.transformIndex],
						cpu->data.levelMaterials[meshInfo.materialIndex].attrib });
					registry.emplace<BlenderName>(drawable, object.blendername);
				}
			}
		});
	}

	inline void Update_GPULevelVulkan(entt::registry& registry, entt::entity entity) {
		//auto& component = registry.get<GPULevel>(entity);
	}

	inline void Destroy_GPULevelVulkan(entt::registry& registry, entt::entity entity) {
		//auto& component = registry.get<GPULevel>(entity);
	}
	
	// MACRO to connect the EnTT GPULevel Logic(s) listed above to the EnTT registry
	CONNECT_COMPONENT_LOGIC() {
		// Connect the logic to the registry for your new component
		registry.on_construct<GPULevelVulkan>().connect<Construct_GPULevelVulkan>();
		registry.on_update<GPULevelVulkan>().connect<Update_GPULevelVulkan>();
		registry.on_destroy<GPULevelVulkan>().connect<Destroy_GPULevelVulkan>();
		// We could do this in main.cpp, but this is more organized
	}
	
} // namespace DRAW