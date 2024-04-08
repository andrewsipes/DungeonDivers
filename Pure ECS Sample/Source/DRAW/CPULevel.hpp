#include "Utility/load_data_oriented.h"

namespace DRAW {
	
	//*** TAGS ***//
	
	//*** COMPONENTS ***//
	struct CPULevel {
		std::string filePath;
		std::string modelPath;
		GW::SYSTEM::GConcurrent async;
		Level_Data data;
	};
	
	//*** SYSTEMS ***//
	
	inline void Construct_CPULevel(entt::registry& registry, entt::entity entity) {
		auto& lvl = registry.get<CPULevel>(entity);

		lvl.async.Create(true);
		lvl.async.BranchSingular([&registry, entity]{
			auto& lvl = registry.get<CPULevel>(entity);
			// on construction, read the file path and load the level
			// save all data from the file into this component, we could split it into multiple components if we wanted
			// don't really see the point in doing that though in this case since its going straight to the GPU & registry
			GW::SYSTEM::GLog log; 
			log.Create("level_load.log");
			log.EnableConsoleLogging(true);

			if (lvl.data.LoadLevel(lvl.filePath.c_str(), lvl.modelPath.c_str(), log) == false)
				std::cout << "Failed to load level: " << lvl.filePath << std::endl;
		});
	}

	inline void Update_CPULevel(entt::registry& registry, entt::entity entity) {
		auto& component = registry.get<CPULevel>(entity);
	}

	inline void Destroy_CPULevel(entt::registry& registry, entt::entity entity) {
		auto& component = registry.get<CPULevel>(entity);
	}
	
	// MACRO to connect the EnTT CPULevel Logic(s) listed above to the EnTT registry
	CONNECT_COMPONENT_LOGIC() {
		// Connect the logic to the registry for your new component
		registry.on_construct<CPULevel>().connect<Construct_CPULevel>();
		registry.on_update<CPULevel>().connect<Update_CPULevel>();
		registry.on_destroy<CPULevel>().connect<Destroy_CPULevel>();
	}
	
} // namespace DRAW