// main entry point for the application

// enables components to define their behaviors locally in an .hpp file
#include "CCL.h"
// include all components, tags, and systems used by this program
#include "DRAW/CPULevel.hpp"
#include "DRAW/VulkanComponents.h"
#include "APP/Window.hpp"

// Local routines for specific application behavior
void GraphicsBehavior(entt::registry& registry);
void GameplayBehavior(entt::registry& registry);
void MainLoopBehavior(entt::registry& registry);

// Architecture is based on components/entities pushing updates to other components/entities (via "patch" function)
int main() {

	// All components, tags, and systems are stored in a single registry
	entt::registry registry;

	// initialize the ECS Component Logic
	CCL::InitializeComponentLogic(registry);

	GraphicsBehavior(registry); // create windows, surfaces, and renderers

	GameplayBehavior(registry); // create entities and components for gameplay
	
	MainLoopBehavior(registry); // update windows and input

	return 0; // now destructors will be called for all components
}

// This function will be called by the main loop to update the graphics
// It will be responsible for loading the Level, creating the VulkanRenderer, and all VulkanInstances
void GraphicsBehavior(entt::registry& registry) {
		
	int windowWidth = 800;
	int windowHeight = 600;

	// Add an entity with a Window component and a VulkanSurface and a VulkanRenderer
	auto display3D = registry.create();
	registry.emplace<APP::Window>(display3D,
		APP::Window{ 300, 300, windowWidth, windowHeight, GW::SYSTEM::GWindowStyle::WINDOWEDBORDERED, "ECS Level Renderer"});

	registry.emplace<DRAW::VulkanSurface>(display3D,
		DRAW::VulkanSurface{ GW::GRAPHICS::DEPTH_BUFFER_SUPPORT, {{{0.75f, 0, 0, 1}}, {1.0f, 0u}} });
	// Keep a handle to the VulkanRenderer so we can give to a pipeline entity
	auto vkRenderer = registry.create();
	registry.emplace<DRAW::VulkanBlenderRender>(display3D, DRAW::VulkanBlenderRender{ display3D });

	/*registry.emplace<GW::GRAPHICS::GVulkanSurface>(vkRenderer,
		registry.get<GW::GRAPHICS::GVulkanSurface>(display3D));
	registry.emplace<GW::SYSTEM::GWindow>(vkRenderer,
		registry.get<GW::SYSTEM::GWindow>(display3D));*/

	// rendering cannot occur without at least one pipeline object
	// these contain the GPU settings we wish to adjust and any shaders+descriptors used to render the models
	auto vkPipeline = registry.create();
	// Attach the shared GW surface to the pipeline
	registry.emplace<GW::GRAPHICS::GVulkanSurface>(vkPipeline,
		registry.get<GW::GRAPHICS::GVulkanSurface>(display3D));

	registry.emplace<DRAW::VulkanPiplineInitialization>(vkPipeline, 
		DRAW::VulkanPiplineInitialization{sizeof(H2B::VERTEX), windowWidth, windowHeight
		});

	registry.emplace<DRAW::VulkanPipeline>(vkPipeline, DRAW::VulkanPipeline{ });
	// attach a vertex and fragment shader to the pipeline (at minimum)
	registry.emplace<DRAW::VulkanVertexShader>(vkPipeline, DRAW::VulkanVertexShader{ "../Shaders/LevelVertex.hlsl" });
	registry.emplace<DRAW::VulkanFragmentShader>(vkPipeline, DRAW::VulkanFragmentShader{ "../Shaders/LevelPixel.hlsl" });
	// registry.emplace<DRAW::VulkanDescriptorSet>(vkPipeline, DRAW::VulkanDescriptorSet{  });
	
	// Once all the pipeline components are created, update the pipeline to create the pipeline
	registry.patch<DRAW::VulkanPipeline>(vkPipeline);

	// Loading a Level will kick off systems that create entities and components
	// All the level data will be loaded onto the CPU and stored for later use/transfer to the GPU
	auto levelOne = registry.create();
	registry.emplace<DRAW::CPULevel>(levelOne,
		DRAW::CPULevel{ "../Levels/GameLevel.txt", "../Models" });
	// attach a GVulkanSurface to the level entity so it can be loaded onto the GPU
	registry.emplace<GW::GRAPHICS::GVulkanSurface>(levelOne,
		registry.get<GW::GRAPHICS::GVulkanSurface>(display3D));
	// Load the level above data onto the GPU by attaching a GPULevel component to the same entity
	// This process will also create renderable entities that the VulkanRenderer can use to draw the scene
	registry.emplace<DRAW::GPULevelVulkan>(levelOne, DRAW::GPULevelVulkan{ });

	// traverse all VulkanInstance entities and assign a pipeline to each one

	// GVulkanSurface will inform us when to release any allocated resources
	// Vulkan components must be removed before the VulkanSurface is invalidated by the closing of the window
	GW::CORE::GEventResponder shutdown;
	shutdown.Create([&](const GW::GEvent& e) {
		GW::GRAPHICS::GVulkanSurface::Events event;
		GW::GRAPHICS::GVulkanSurface::EVENT_DATA data;
		if (+e.Read(event, data) && event == GW::GRAPHICS::GVulkanSurface::Events::RELEASE_RESOURCES) {
			// clear all Vulkan entities and components from the registry
			// invokes on_destroy() for all components that have it
			// we do this now so that the VulkanSurface will still be valid while it happens
			registry.clear<DRAW::VulkanVertexShader>();
			registry.clear<DRAW::VulkanFragmentShader>();
			registry.clear<DRAW::VulkanPipeline>();
			registry.clear<DRAW::VulkanBlenderRender>();
			registry.clear<DRAW::GPULevelVulkan>();
		}
	});
	registry.get<GW::GRAPHICS::GVulkanSurface>(display3D).Register(shutdown);
	registry.emplace<GW::CORE::GEventResponder>(display3D, shutdown.Relinquish());
}

// This function will be called by the main loop to update the gameplay
// It will be responsible for updating the VulkanInstances and any other gameplay components
void GameplayBehavior(entt::registry& registry) {
	
	// Gameplay Style Code Test (what you might find in gameplay components)

		// find a VulkanInstance by its Blender name and update its position

		// find a VulkanInstance by its name and remove it

		// find a VulkanInstance by its name and duplicate it

	// End Gameplay Style Code Test
}

// This function will be called by the main loop to update the main loop
// It will be responsible for updating any created windows and handling any input
void MainLoopBehavior(entt::registry& registry) {
	
	// main loop
	int closedCount; // count of closed windows
	auto winView = registry.view<APP::Window>(); // for updating all windows
	do {
		closedCount = 0;
		// find all Windows that are not closed and call "patch" to update them
		for (auto entity : winView) {
			if (registry.any_of<APP::WindowClosed>(entity))
				++closedCount;
			else
				registry.patch<APP::Window>(entity); // calls on_update()
		}
	} while (winView.size() != closedCount); // exit when all windows are closed
}
