namespace APP {
	
	//*** TAGS ***//
	struct WindowClosed {};
	
	//*** COMPONENTS ***//

	// Window component
	struct Window {
		int x = 0, y = 0, width = 100, height = 100;
		GW::SYSTEM::GWindowStyle style = GW::SYSTEM::GWindowStyle::WINDOWEDBORDERED;
		std::string title = "Uninitialized";
	};

	//*** SYSTEMS ***//
	
	// run this code when a Window component is created
	inline void Construct_Window(entt::registry& registry, entt::entity entity) {
		
		auto& window = registry.get<Window>(entity);
		GW::SYSTEM::GWindow newWindow;
		if (+newWindow.Create(window.x, window.y, window.width, window.height, window.style)) {
			newWindow.SetWindowName(window.title.c_str());
			registry.emplace<GW::SYSTEM::GWindow>(entity, newWindow.Relinquish());
		}
		else 
			std::cout << "Failed to create window" << std::endl;
	}

	// run this code when a Window component is updated
	inline void Update_Window(entt::registry& registry, entt::entity entity) {

		auto& win = registry.get<GW::SYSTEM::GWindow>(entity);
		if (-win.ProcessWindowEvents()) {
			registry.emplace_or_replace<WindowClosed>(entity);
		}
		// update any 3D graphics surfaces that are attached to this window
		else if (registry.any_of<DRAW::VulkanSurface>(entity)) {
			// update the window's GVulkanSurface if it has one
			registry.patch<DRAW::VulkanSurface>(entity);
		}
		// this example focuses on Vulkan, but the same logic could be applied to DirectX12
		else if (registry.any_of<GW::GRAPHICS::GDirectX12Surface>(entity)) {
			// update the window's GDirectX12Surface if it has one
			registry.patch<GW::GRAPHICS::GDirectX12Surface>(entity);
		}
		// ... add more 3D graphics surfaces here (2 is probably plenty for most engines)
	}
	
	// Use this MACRO to connect the EnTT Component Logic
	CONNECT_COMPONENT_LOGIC() {
		// register the Window component's logic
		registry.on_construct<Window>().connect<Construct_Window>();
		registry.on_update<Window>().connect<Update_Window>();
	}

} // namespace APP