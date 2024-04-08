namespace DRAW
{
	inline void Construct_CPULevel(entt::registry& registry, entt::entity entity) {
		auto& lvl = registry.get<CPULevel>(entity);

		lvl.async.Create(true);
		//lvl.async.BranchSingular([&registry, entity] {
			//auto& lvl = registry.get<CPULevel>(entity);
			// on construction, read the file path and load the level
			// save all data from the file into this component, we could split it into multiple components if we wanted
			// don't really see the point in doing that though in this case since its going straight to the GPU & registry
			GW::SYSTEM::GLog log;
			log.Create("level_load.log");
			log.EnableConsoleLogging(false); // Enable to print level loading to console

			if (lvl.data.LoadLevel(lvl.filePath.c_str(), lvl.modelPath.c_str(), log) == false)
				std::cout << "Failed to load level: " << lvl.filePath << std::endl;
			//});
	}

	// MACRO to connect the EnTT CPULevel Logic(s) listed above to the EnTT registry
	CONNECT_COMPONENT_LOGIC() {
		// Connect the logic to the registry for your new component
		registry.on_construct<CPULevel>().connect<Construct_CPULevel>();
	}

} // namespace DRAW