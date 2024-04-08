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

	// ANOTHER APPROACH:
	// we also add macros for runnning systems, shutting down systems and creating entities
	// currently everything is so piece meal is slows down the development process
	// essentially we need a way to run larger code in addition to just individual components
	// Organization: MODULE_FOLDER->Components->Systems->Behaviors/Entities
	// Example: RENDERING->components.h->signals.hpp->systems.hpp->entities.hpp
	// Files Inside:
	// Rendering.h defines module. includes components.h, includes .hpps within RENDERING namespace
	// components.h: All components used/defined by this module, included by other modules
	// signals.hpp: defines EnTT signals(construct/update/destroy) used for logic not met by general systems
	// systems.hpp: code that runs every frame on combinations of components related to this module 
	// entities.hpp: runs during initialization and creates the various initial entities to run the module    
	// Make a template version of above

	// Execute all the stored component logic to register components and systems
	inline void InitializeComponentLogic(entt::registry& registry) {
		for (auto& logic : componentLogic) {
			logic(registry);
		}
	}
   
} // namespace CCL