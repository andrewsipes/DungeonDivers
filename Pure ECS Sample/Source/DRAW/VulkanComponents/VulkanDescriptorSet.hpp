// CmpTmp.hpp is a template file for defining a new component and its logic.

// TODO: Change MODULE to your module name
namespace MODULE {
	
	//*** TAGS ***//
	// TODO: Add any custom tags you need here

	//*** COMPONENTS ***//
	// TODO: Add your writable component(s) here

	//*** SYSTEMS ***//
	// TODO: Add your system(s) here (a.k.a component logic)
	
	// Boilerplate for logic that runs when adding a new component to the registry
	inline void Construct_Component(entt::registry& registry, entt::entity entity) {
		auto& component = registry.get<Component>(entity);
	}

	// Boilerplate for logic that runs when updating a component via "patch"
	inline void Update_Component(entt::registry& registry, entt::entity entity) {
		auto& component = registry.get<Component>(entity);
	}

	// Boilerplate for logic that runs when removing a component from the registry
	inline void Destroy_Component(entt::registry& registry, entt::entity entity) {
		auto& component = registry.get<Component>(entity);
	}
	
	// MACRO to connect the EnTT Component Logic(s) listed above to the EnTT registry
	CONNECT_COMPONENT_LOGIC() {
		// Connect the logic to the registry for your new component
		registry.on_construct<Component>().connect<Construct_Component>();
		registry.on_update<Component>().connect<Update_Component>();
		registry.on_destroy<Component>().connect<Destroy_Component>();
		// We could do this in main.cpp, but this is more organized
	}
	

} // namespace MODULE