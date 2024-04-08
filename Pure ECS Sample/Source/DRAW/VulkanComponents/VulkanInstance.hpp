namespace DRAW { // Not sure if we even need this component...
	
	//*** TAGS ***//
	
	//*** COMPONENTS ***//
	struct VulkanInstance {
		H2B::BATCH DrawBatch;
	};
	
	//*** SYSTEMS ***//
	
	inline void Construct_VulkanInstance(entt::registry& registry, entt::entity entity) {
		auto& component = registry.get<VulkanInstance>(entity);
	}

	inline void Update_VulkanInstance(entt::registry& registry, entt::entity entity) {
		auto& component = registry.get<VulkanInstance>(entity);
	}

	inline void Destroy_VulkanInstance(entt::registry& registry, entt::entity entity) {
		auto& component = registry.get<VulkanInstance>(entity);
	}
	
	// MACRO to connect the EnTT VulkanInstance Logic(s) listed above to the EnTT registry
	CONNECT_COMPONENT_LOGIC() {
		// Connect the logic to the registry for your new component
		registry.on_construct<VulkanInstance>().connect<Construct_VulkanInstance>();
		registry.on_update<VulkanInstance>().connect<Update_VulkanInstance>();
		registry.on_destroy<VulkanInstance>().connect<Destroy_VulkanInstance>();
	}
	
} // namespace DRAW