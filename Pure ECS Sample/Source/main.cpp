// main entry point for the application

// enables components to define their behaviors locally in an .hpp file
#include "CCL.h"
// include modules in the order that they are dependent on each other
#include "APP/Window.hpp"
#include "DRAW/VulkanSurface.hpp"

// Architecture is based on components/entities pushing updates to other components/entities (via "patch" function)
int main() {

	// All components, tags, and systems are stored in a single registry
	entt::registry registry;

	// initialize the ECS Component Logic
	CCL::InitializeComponentLogic(registry);

	// Add an entity with a Window component, a VulkanSurface and a Geometry(triangle) component to the registry
	auto entity = registry.create();
	registry.emplace<APP::Window>(entity, 
		APP::Window{ 300, 300, 800, 600, GW::SYSTEM::GWindowStyle::WINDOWEDBORDERED, "ECS Triangle"});
	registry.emplace<DRAW::VulkanSurface>(entity, 
		DRAW::VulkanSurface{ GW::GRAPHICS::DEPTH_BUFFER_SUPPORT, {{{0.75f, 0, 0, 1}}, {1.0f, 0u}} });
	//registry.emplace<Geometry>(entity);

	// main loop
	auto closedView = registry.view<APP::WindowClosed>(); // count all closed windows
	auto winView = registry.view<GW::SYSTEM::GWindow>(); // for updating all windows
	do {
		// find all GWindows that are not closed and call "patch" to update them
		for (auto entity : winView) {
			registry.patch<GW::SYSTEM::GWindow>(entity); // calls on_update()
		}
	} while (winView.size() != closedView.size()); // exit when all windows are closed

	return 0;
}
