// Enables inline ECS component logic and system logic to be added to the ECS system
namespace CCL {

	// Component logic functions
	std::list<std::function<void(entt::registry& reg)>> componentLogic;

	// struct which contains logic to add to the componentLogic list
	struct ComponentLogic {
		ComponentLogic(std::function<void(entt::registry& reg)> logic) {
			componentLogic.push_back(logic);
		}
	};

// Allows logic to be added to the componentLogic list (idea from Catch2)
#define INTERNAL_CONNECT_COMPONENT_LOGIC( LogicFunction ) \
        static void LogicFunction(entt::registry& registry); \
        namespace{ CCL::ComponentLogic _StoreComponent##__COUNTER__(LogicFunction); } \
        static void LogicFunction(entt::registry& registry)
#define CONNECT_COMPONENT_LOGIC() \
        INTERNAL_CONNECT_COMPONENT_LOGIC( _DefineLogic##__COUNTER__ )

	// Execute all the stored component logic to register components and systems
	inline void InitializeComponentLogic(entt::registry& registry) {
		for (auto& logic : componentLogic) {
			logic(registry);
		}
	}
   
} // namespace CCL