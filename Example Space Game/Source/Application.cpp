#include "./Application.h"
#include "./renderer.h"


using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;

bool Application::Init() 
{
	eventPusher.Create();


	// load all game settigns
	gameConfig = std::make_shared<GameConfig>(); 
	// create the ECS system
	game = std::make_shared<flecs::world>(); 
	// init all other systems
	if (InitWindow() == false) 
		return false;
	if (InitInput() == false)
		return false;
	if (InitAudio() == false)
		return false;
	//if (InitGraphics() == false)
	//	return false;
	if (InitEntities() == false)
		return false;
	//if (InitSystems() == false)
	//	return false;
	return true;
}

//bool Application::Run() 
//{
//	
// 
// 
// ClearValue clrAndDepth[2];
//	clrAndDepth[0].color = { {0, 0, 0, 1} };
//	clrAndDepth[1].depthStencil = { 1.0f, 0u };
//	// grab vsync selection
//	bool vsync = gameConfig->at("Window").at("vsync").as<bool>();
//	// set background color from settings
//	const char* channels[] = { "red", "green", "blue" };
//	for (int i = 0; i < std::size(channels); ++i) {
//		clrAndDepth[0].color.float32[i] =
//			gameConfig->at("BackGroundColor").at(channels[i]).as<float>();
//	}
//	// create an event handler to see if the window was closed early
//	bool winClosed = false;
//	GW::CORE::GEventResponder winHandler;
//	winHandler.Create([&winClosed](GW::GEvent e) {
//		GW::SYSTEM::GWindow::Events ev;
//		if (+e.Read(ev) && ev == GW::SYSTEM::GWindow::Events::DESTROY)
//			winClosed = true;
//	});	
//	window.Register(winHandler);
//	while (+window.ProcessWindowEvents())
//	{
//		if (winClosed == true)
//			return true;
//		if (+vulkan.StartFrame(2, clrAndDepth))
//		{
//			if (GameLoop() == false) {
//				vulkan.EndFrame(vsync);
//				return false;
//			}
//			if (-vulkan.EndFrame(vsync)) {
//				// failing EndFrame is not always a critical error, see the GW docs for specifics
//			}
//		}
//		else
//			return false;
//	}
//	return true;
//}

bool Application::Run() {

	GEventResponder msgs;
	float clr[] = { 194.0f / 255.0f, 51.0f / 255.0f, 29.0f / 255.0f, 1 }; // Buffer

		msgs.Create([&](const GW::GEvent& e) {
			GW::SYSTEM::GWindow::Events q;
			if (+e.Read(q) && q == GWindow::Events::RESIZE)
				clr[2] += 0.01f;
			});
		win.Register(msgs);

		if (+ogl.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
		{
			QueryOGLExtensionFunctions(ogl); // Link Needed OpenGL API functions
			RendererManager rendererManager(win, ogl);

			while (+win.ProcessWindowEvents())
			{
				glClearColor(clr[0], clr[1], clr[2], clr[3]);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				//Update camera then render
				rendererManager.UpdateCamera(gameConfig->at("Window").at("width").as<int>(), gameConfig->at("Window").at("height").as<int>());
				rendererManager.Render();
				ogl.UniversalSwapBuffers();


			}
		}
	//}
	return 0;

}

bool Application::Shutdown() 
{
	// disconnect systems from global ECS
	if (playerSystem.Shutdown() == false)
		return false;
	if (levelSystem.Shutdown() == false)
		return false;
	//if (vkRenderingSystem.Shutdown() == false)
		//return false;
	if (physicsSystem.Shutdown() == false)
		return false;
	if (bulletSystem.Shutdown() == false)
		return false;
	if (enemySystem.Shutdown() == false)
		return false;

	return true;
}
bool Application::InitWindow()
{
	// grab settings
	int width = gameConfig->at("Window").at("width").as<int>();
	int height = gameConfig->at("Window").at("height").as<int>();
	int xstart = gameConfig->at("Window").at("xstart").as<int>();
	int ystart = gameConfig->at("Window").at("ystart").as<int>();
	std::string title = gameConfig->at("Window").at("title").as<std::string>();
	// open window
	if (+win.Create(xstart, ystart, width, height, GWindowStyle::WINDOWEDLOCKED) &&
		+win.SetWindowName(title.c_str())) {
		return true;
	}
	return false;
}

bool Application::InitInput()
{
	if (-gamePads.Create())
		return false;
	if (-immediateInput.Create(win))
		return false;
	if (-bufferedInput.Create(win))
		return false;
	return true;
}

bool Application::InitAudio()
{
	if (-audioEngine.Create())
		return false;
	return true;
}

//bool Application::InitGraphics()
//{
//#ifndef NDEBUG
//	const char* debugLayers[] = {
//		"VK_LAYER_KHRONOS_validation", // standard validation layer
//		//"VK_LAYER_RENDERDOC_Capture" // add this if you have installed RenderDoc
//	};
//	if (+vulkan.Create(window, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT,
//		sizeof(debugLayers) / sizeof(debugLayers[0]),
//		debugLayers, 0, nullptr, 0, nullptr, false))
//		return true;
//#else
//	if (+vulkan.Create(window, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
//		return true;
//#endif
//	return false;
//}

bool Application::InitEntities()
{
	// Load bullet prefabs
	if (weapons.Load(game, gameConfig, audioEngine) == false)
		return false;
	// Load the player entities
	if (players.Load(game, gameConfig) == false)
		return false;
	// Load the enemy entities
	if (enemies.Load(game, gameConfig, audioEngine) == false)
		return false;

	return true;
}

//bool Application::InitSystems()
//{
//	// connect systems to global ECS
//	if (playerSystem.Init(	game, gameConfig, immediateInput, bufferedInput, 
//							gamePads, audioEngine, eventPusher) == false)
//		return false;
//	if (levelSystem.Init(game, gameConfig, audioEngine) == false)
//		return false;
//	if (vkRenderingSystem.Init(game, gameConfig, vulkan, window) == false)
//		return false;
//	if (physicsSystem.Init(game, gameConfig) == false)
//		return false;
//	if (bulletSystem.Init(game, gameConfig) == false)
//		return false;
//	if (enemySystem.Init(game, gameConfig, eventPusher) == false)
//		return false;
//
//	return true;
//}

bool Application::GameLoop()
{
	// compute delta time and pass to the ECS system
	static auto start = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration<double>(
		std::chrono::steady_clock::now() - start).count();
	start = std::chrono::steady_clock::now();
	// let the ECS system run
	return game->progress(static_cast<float>(elapsed)); 
}
