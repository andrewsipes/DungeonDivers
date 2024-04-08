namespace DRAW {
	//*** TAGS ***//
	
	//*** COMPONENTS ***//

	struct VulkanSurface {
		GW::GRAPHICS::GGraphicsInitOptions surfaceOptions;
		VkClearValue clrAndDepth[2];
	};

	//*** SYSTEMS ***//
	
	// Surface creation
	ECS::add_init_system initSurface( ECS::InitPriority::init, [](entt::registry& reg) {
		// find any GWindows that need a VulkanSurface and add a real one
		auto view = reg.view<VulkanSurface, const GW::SYSTEM::GWindow>(
			entt::exclude<GW::GRAPHICS::GVulkanSurface>); // ignore any that already have one
		for (auto entity : view) {
			auto& window = view.get<GW::SYSTEM::GWindow>(entity);
			auto& settings = view.get<VulkanSurface>(entity);
			GW::GRAPHICS::GVulkanSurface newSurface;
			if (-newSurface.Create(window, settings.surfaceOptions))
				return false;
			reg.emplace<GW::GRAPHICS::GVulkanSurface>(entity, newSurface.Relinquish());
		}
		return true;
	});

	// Processing the Vulkan Surface

	// Clear the surface & start the frame
	ECS::add_continuous_system beginFrame(ECS::RunPriority::preRender, [](entt::registry& reg) {
		// clear any vulkan surfaces we find each frame
		auto view = reg.view<VulkanSurface, GW::GRAPHICS::GVulkanSurface>();
		for (auto entity : view) {
			auto& settings = view.get<VulkanSurface>(entity);
			auto& surface = view.get<GW::GRAPHICS::GVulkanSurface>(entity);
			if (-surface.StartFrame(2, settings.clrAndDepth)) {
				return false; // exit if we can't start a frame
			}
		}
		return true;
	});

	// End the frame & present the surface
	ECS::add_continuous_system endFrame(ECS::RunPriority::postRender, [](entt::registry& reg) {
		// present any vulkan surfaces we find each frame
		auto view = reg.view<GW::GRAPHICS::GVulkanSurface>();
		for (auto entity : view) {
			auto& surface = view.get<GW::GRAPHICS::GVulkanSurface>(entity);
			// end frame may fail in a non-fatal way, so we don't abort if it does
			surface.EndFrame(true);
		}
		return true;
	});

} // namespace DRAW