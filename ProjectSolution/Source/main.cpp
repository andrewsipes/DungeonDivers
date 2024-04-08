// main entry point for the application
// enables components to define their behaviors locally in an .hpp file
#include "CCL.h"
#include "UTIL/Utilities.h"
// include all components, tags, and systems used by this program
#include "DRAW/DrawComponents.h"
#include "DRAW/VulkanBuffers.hpp"
#include "DRAW/CPULevel.hpp"
#include "DRAW/VulkanGPULevel.hpp"
#include "DRAW/VulkanRenderer.hpp"
#include "APP/Window.hpp"



// Local routines for specific application behavior
void GraphicsBehavior(entt::registry& registry);
void GameplayBehavior(entt::registry& registry);
void MainLoopBehavior(entt::registry& registry);

// Architecture is based on components/entities pushing updates to other components/entities (via "patch" function)
int main()
{

	// All components, tags, and systems are stored in a single registry
	entt::registry registry;

	// initialize the ECS Component Logic
	CCL::InitializeComponentLogic(registry);

	// Setup our Gateware proxies
	UTIL::matrixProxy.Create();

	GraphicsBehavior(registry); // create windows, surfaces, and renderers

	GameplayBehavior(registry); // create entities and components for gameplay
	
	MainLoopBehavior(registry); // update windows and input

	// clear all entities and components from the registry
	// invokes on_destroy() for all components that have it
	// registry will still be intact while this is happening
	registry.clear(); 

	return 0; // now destructors will be called for all components
}

// This function will be called by the main loop to update the graphics
// It will be responsible for loading the Level, creating the VulkanRenderer, and all VulkanInstances
void GraphicsBehavior(entt::registry& registry)
{
	int windowWidth = 800;
	int windowHeight = 600;

	// Add an entity to handle all the graphics data
	auto display = registry.create();
	registry.emplace<APP::Window>(display,
		APP::Window{ 300, 300, windowWidth, windowHeight, GW::SYSTEM::GWindowStyle::WINDOWEDBORDERED, "Dev4Now" });

	registry.emplace<DRAW::VulkanRendererInitialization>(display,
		DRAW::VulkanRendererInitialization{ 
			"../Shaders/LevelVertex.hlsl", "../Shaders/LevelPixel.hlsl",
			{ {0.2f, 0.2f, 0.25f, 1} } , { 1.0f, 0u }, 65.f, 0.1f, 100.0f });
	registry.emplace<DRAW::VulkanRenderer>(display);
	
	// TODO : Load the Level then update the Vertex and Index Buffers
	//{
	registry.emplace<DRAW::CPULevel>(display,
		DRAW::CPULevel{ "../Levels/GameLevel.txt", "../Models" });

	registry.emplace<DRAW::GPULevel>(display, DRAW::GPULevel{ });
	//}

	// Register for Vulkan clean up
	GW::CORE::GEventResponder shutdown;
	shutdown.Create([&](const GW::GEvent& e) {
		GW::GRAPHICS::GVulkanSurface::Events event;
		GW::GRAPHICS::GVulkanSurface::EVENT_DATA data;
		if (+e.Read(event, data) && event == GW::GRAPHICS::GVulkanSurface::Events::RELEASE_RESOURCES) {
			registry.clear<DRAW::VulkanRenderer>();
		}
		});
	registry.get<DRAW::VulkanRenderer>(display).vlkSurface.Register(shutdown);
	registry.emplace<GW::CORE::GEventResponder>(display, shutdown.Relinquish());

	// Create a camera entity and emplace it
	auto camera = registry.create();
	GW::MATH::GMATRIXF initialCamera;
	GW::MATH::GVECTORF translate = { 0.0f,  5.0f, -10.0f };
	GW::MATH::GVECTORF lookat = { 0.0f, 0.0f, 0.0f };
	GW::MATH::GVECTORF up = { 0.0f, 1.0f, 0.0f };
	UTIL::matrixProxy.TranslateGlobalF(initialCamera, translate, initialCamera);
	UTIL::matrixProxy.LookAtLHF(translate, lookat, up, initialCamera);
	// Inverse to turn it into a camera matrix, not a view matrix. This will let us do
	// camera manipulation in the component easier
	UTIL::matrixProxy.InverseF(initialCamera, initialCamera);
	registry.emplace<DRAW::Camera>(camera,
		DRAW::Camera{ initialCamera });
}

// This function will be called by the main loop to update the gameplay
// It will be responsible for updating the VulkanInstances and any other gameplay components
void GameplayBehavior(entt::registry& registry)
{
	
	// Gameplay Style Code Test (what you might find in gameplay components)

		// find a VulkanInstance by its Blender name and update its position

		// find a VulkanInstance by its name and remove it

		// find a VulkanInstance by its name and duplicate it

	// End Gameplay Style Code Test
}

// This function will be called by the main loop to update the main loop
// It will be responsible for updating any created windows and handling any input
void MainLoopBehavior(entt::registry& registry)
{	
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
