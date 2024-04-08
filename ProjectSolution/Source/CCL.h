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
#define CCL_INTERNAL_EXPAND( x, y ) x##y
#define CCL_INTERNAL_COMBINE( left, right ) CCL_INTERNAL_EXPAND( left, right )
#define CCL_INTERNAL_CONNECT_COMPONENT_LOGIC( function ) \
        static void function( entt::registry& registry ); \
        namespace{ CCL::ComponentLogic CCL_INTERNAL_COMBINE( _StoreComponent, __COUNTER__ )( function ); } \
        static void function( entt::registry& registry )
// This is the part of the macro series that the user will actually use
#define CONNECT_COMPONENT_LOGIC() \
        CCL_INTERNAL_CONNECT_COMPONENT_LOGIC( CCL_INTERNAL_COMBINE( _DefineLogic, __COUNTER__ ) )

	// Execute all the stored component logic to register components and systems
	inline void InitializeComponentLogic(entt::registry& registry) {
		for (auto& logic : componentLogic) {
			logic(registry);
		}
	}
   
} // namespace CCL