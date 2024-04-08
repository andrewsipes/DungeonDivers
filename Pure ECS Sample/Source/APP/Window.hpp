namespace APP {
	//*** TAGS ***//
	
	//*** COMPONENTS ***//

	// Window component
	struct Window {
		int x, y, width, height;
		GW::SYSTEM::GWindowStyle style;
		std::string title;
	};

	//*** SYSTEMS ***//

	// Window creation system
	ECS::add_init_system initWindow( ECS::InitPriority::init, [](entt::registry& reg) {
		// find all Windows and create them, adding them to the existing entity
		auto view = reg.view<Window>(entt::exclude<GW::SYSTEM::GWindow>);
		for (auto entity : view) {
			auto& window = view.get<Window>(entity);
			GW::SYSTEM::GWindow newWindow;
			if (-newWindow.Create(window.x, window.y, window.width, window.height, window.style))
				return false;
			newWindow.SetWindowName(window.title.c_str());
			reg.emplace<GW::SYSTEM::GWindow>(entity, newWindow.Relinquish());
		}
		return true;
	});

	// Window processing system
	ECS::add_continuous_system processWindow( ECS::RunPriority::preUpdate, [](entt::registry& reg) {
		// update all GWindows
		auto view = reg.view<GW::SYSTEM::GWindow>();
		for (auto entity : view) {
			auto& win = view.get<GW::SYSTEM::GWindow>(entity);
			if (-win.ProcessWindowEvents()) {
				reg.destroy(entity);
			}
		}
		return !view.empty();
	});

} // namespace APP