namespace DRAW {
	
	//*** TAGS ***//
	
	//*** COMPONENTS ***//

	struct VulkanBlenderRender {
		entt::entity window_and_surface;
	};
	
	//*** SYSTEMS ***//
	
	inline void Construct_VulkanBlenderRender(entt::registry& registry, entt::entity entity) {
		auto& renderer = registry.get<VulkanBlenderRender>(entity);
		// some of this part will need to be split into a separate descriptor set component
		// I am just tossing this here for now to make sure the core logic will work


	}

	inline void Update_VulkanBlenderRender(entt::registry& registry, entt::entity entity) {
		auto& component = registry.get<VulkanBlenderRender>(entity);
		// grab the device and command list from the VulkanSurface

		// sort by H2B::BATCH (then pipeline when available)
		// copy all GPU_INSTANCE data into the storage buffer cpu side vector (clear first)
		// when a differing H2B::BATCH is found, note the offset and call cmd->draw with previous BATCH

		// write changes to storage buffer (patch)
	}

	inline void Destroy_VulkanBlenderRender(entt::registry& registry, entt::entity entity) {
		auto& component = registry.get<VulkanBlenderRender>(entity);
	}
	
	// MACRO to connect the EnTT VulkanRenderer Logic(s) listed above to the EnTT registry
	CONNECT_COMPONENT_LOGIC() {
		// Connect the logic to the registry for your new component
		registry.on_construct<VulkanBlenderRender>().connect<Construct_VulkanBlenderRender>();
		registry.on_update<VulkanBlenderRender>().connect<Update_VulkanBlenderRender>();
		registry.on_destroy<VulkanBlenderRender>().connect<Destroy_VulkanBlenderRender>();
	}

} // namespace DRAW