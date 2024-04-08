// main entry point for the application

// define the priorities of the applications systems
#include "priority.h"
// include the main registry next so all modules can use it
#include "registry.h"
// include modules in the order that they are dependent on each other
#include "APP/Window.hpp"
#include "DRAW/VulkanSurface.hpp"

int main() {

	// Add an entity with a Window component, a VulkanSurface and a Geometry(triangle) component to the registry
	auto entity = ECS::registry.create();
	ECS::registry.emplace<APP::Window>(entity, 
		APP::Window{ 300, 300, 800, 600, GW::SYSTEM::GWindowStyle::WINDOWEDBORDERED, "ECS Triangle"});
	ECS::registry.emplace<DRAW::VulkanSurface>(entity, 
		DRAW::VulkanSurface{ GW::GRAPHICS::DEPTH_BUFFER_SUPPORT, {{{0.75f, 0, 0, 1}}, {1.0f, 0u}} });
	//registry.emplace<Geometry>(entity);

	// Logic for initializing and updating the various components are handled by their own systems
	// The combination of various components are what create the behavior of the application (ECS Design)
	
	// Program will run until one of the continuous systems returns false
	while (ECS::IterateSystems()) { }
	ECS::ShutdownSystems();

	return 0;
}
