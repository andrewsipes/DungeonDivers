namespace APP {
	//*** TAGS ***//
	struct WindowClosed {};
	
	//*** COMPONENTS ***//

	// Window component
	struct Window {
		int x, y, width, height;
		GW::SYSTEM::GWindowStyle style;
		std::string title;
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
	inline void Update_GWindow(entt::registry& registry, entt::entity entity) {

		auto& win = registry.get<GW::SYSTEM::GWindow>(entity);
		if (-win.ProcessWindowEvents()) {
			registry.emplace_or_replace<WindowClosed>(entity);
		}
		else if (registry.try_get<GW::GRAPHICS::GVulkanSurface>(entity)) {
			// update the window's GVulkanSurface if it has one
			registry.patch<GW::GRAPHICS::GVulkanSurface>(entity);
		}
	}
	
	// Use this MACRO to connect the EnTT Component Logic
	CONNECT_COMPONENT_LOGIC()
	{
		// register the Window component's logic
		registry.on_construct<Window>().connect<Construct_Window>();
		registry.on_update<GW::SYSTEM::GWindow>().connect<Update_GWindow>();
		// add on_destroy if needed
	}

} // namespace APP