namespace DRAW {
	
	//*** TAGS ***//
	
	//*** COMPONENTS ***//

	struct VulkanSurface {
		GW::GRAPHICS::GGraphicsInitOptions surfaceOptions;
		VkClearValue clrAndDepth[2];
	};

	//*** SYSTEMS ***//
	
	// Surface creation
	inline void Construct_VulkanSurface(entt::registry& registry, entt::entity entity) {
		// create the Vulkan surface with the settings provided
		auto& window = registry.get<GW::SYSTEM::GWindow>(entity);
		auto& settings = registry.get<VulkanSurface>(entity);
		GW::GRAPHICS::GVulkanSurface newSurface;
		if (+newSurface.Create(window, settings.surfaceOptions)) {
			registry.emplace<GW::GRAPHICS::GVulkanSurface>(entity, newSurface.Relinquish());
		}
		else 
			std::cout << "Failed to create Vulkan Surface" << std::endl;
	}

	// Processing the Vulkan Surface
	inline void Update_VulkanSurface(entt::registry& registry, entt::entity entity) {
		// clear any Vulkan surfaces we find each frame
		auto& settings = registry.get<VulkanSurface>(entity);
		auto& surface = registry.get<GW::GRAPHICS::GVulkanSurface>(entity);
		if (+surface.StartFrame(2, settings.clrAndDepth)) {
			// Look for an attached VulkanRenderer and update it here (patch)
			// ...

			// end frame may fail in a non-fatal way, so we don't abort if it does
			if(-surface.EndFrame(true))
				std::cout << "Failed to end frame" << std::endl;
		}
		else
			std::cout << "Failed to start frame" << std::endl;		
	}

	// Use this MACRO to connect the EnTT Component Logic
	CONNECT_COMPONENT_LOGIC() {
		// register the Window component's logic
		registry.on_construct<VulkanSurface>().connect<Construct_VulkanSurface>();
		registry.on_update<VulkanSurface>().connect<Update_VulkanSurface>();
	}

} // namespace DRAW