#include <map>
#include <string>
#include "./Application.h"
#include "./renderer.h"
#include "../gateware-main/Gateware.h"

using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;
using namespace GW::AUDIO;

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
	if (InitSystems() == false)
		return false;
	return true;
}

//bool Application::Run()
//{
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
	leftMouse = false;
	running = true;
	GEventResponder msgs;
	GW::SYSTEM::GLog log;
	log.Create("output.txt");
	auto lvl = std::make_shared<Level_Objects>();
	auto lvl2 = std::make_shared<Level_Objects>();
	float clr[] = { gameConfig->at("BackGroundColor").at("red").as<float>(), gameConfig->at("BackGroundColor").at("blue").as<float>(), gameConfig->at("BackGroundColor").at("green").as<float>(), 1 }; // Buffer
	lvl->LoadMeshes("../MainMenu.txt", "../Models/MainMenuModels", log.Relinquish());
	//lvl2->LoadMeshes("../Models/TestWorld/Level2/GameLevel.txt", "../Models/TestWorld/Level2/Models", log.Relinquish());
	//lvl2->LoadMeshes("../Models/enemytestlvl/GameLevel.txt", "../Models/enemytestlvl/Models", log.Relinquish());
	//lvl2->LoadMeshes("../Models/Level2/Level2.txt", "../Models/Level2/Level2Models", log.Relinquish());
	lvl2->LoadMeshes("../Models/Level3/Level3.txt", "../Models/Level3/Level3Models", log.Relinquish());

		msgs.Create([&](const GW::GEvent& e) {
			GW::SYSTEM::GWindow::Events q;
			if (+e.Read(q) && q == GWindow::Events::RESIZE)
				clr[2] += 0.01f;
		});
	win.Register(msgs);

	if (+ogl.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
	{
		QueryOGLExtensionFunctions(ogl); // Link Needed OpenGL API functions
		RendererManager rendererManager(win, ogl, *gameConfig, *this, *lvl);
		PlayerStats playerStats(*gameConfig);
		auto& mainMenuMusic = musicTracks["MainMenu"];
		mainMenuMusic.Play(true);
		AddEntities(lvl2, game);
		AddSystems(lvl2, game, gameConfig, gInput, bufferedInput, gamePads, audioEngine, eventPusher, &playerStats, &rendererManager);

		while (+win.ProcessWindowEvents() && running == true)
		{
			lvl2->Update(game, lvl2);

			if(!rendererManager.pauseMenu->render && !rendererManager.isPauseMenuRendered)
				GameLoop();

			glClearColor(clr[0], clr[1], clr[2], clr[3]);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			#ifdef NDEBUG
				if (rendererManager.mainMenuHUD->render){
					rendererManager.freecam = false;
				}
			#endif

			rendererManager.UpdateCamera(gameConfig->at("Window").at("width").as<int>(), gameConfig->at("Window").at("height").as<int>());
			rendererManager.Render();

			//event Handling for the mainMenu - starts the game
			if (rendererManager.mainMenuHUD->render) {

				if (rendererManager.mainMenuHUD->startButton->HandleInputLeftMouseButton(gInput)) {
					leftMouse = true;
					rendererManager.mainMenuHUD->toggleRender();
					rendererManager.playerHUD->toggleRender();
					rendererManager.changeLevel(*lvl2);
				}

			}

			//Return Left Mouse state for re-use
			else if ((leftMouse) && !(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
				leftMouse = false;
			}

			//LEVEL SWAP: Currently works by using 0 or 1
			{
				//use these to determine if flag is read
				bool zero = false, one = false;

				//Main Menu
				if (!zero && (GetAsyncKeyState(0x30) & 0x8000)) {
					zero = true;

					rendererManager.changeLevel(*lvl);

				}

				else if (zero && !(GetAsyncKeyState(0x30) & 0x8000)) {
					zero = false;
				}

				//Level1
				if (!one && (GetAsyncKeyState(0x31) & 0x8000)) {
					one = true;

					rendererManager.changeLevel(*lvl2);

				}

				else if (zero && !(GetAsyncKeyState(0x30) & 0x8000)) {
					one = false;
				}
			}


			ogl.UniversalSwapBuffers();

		}
	}
	return 0;

}

bool Application::Shutdown()
{
	// disconnect systems from global ECS
	//if (playerSystem.Shutdown() == false)
	//	return false;
	//if (levelSystem.Shutdown() == false)
	//	return false;
	////if (vkRenderingSystem.Shutdown() == false)
	//	//return false;
	//if (physicsSystem.Shutdown() == false)
	//	return false;
	//if (bulletSystem.Shutdown() == false)
	//	return false;
	//if (enemySystem.Shutdown() == false)
	//	return false;
	running = false;
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
	if (+win.Create(xstart, ystart, width, height, GWindowStyle::WINDOWEDBORDERED) &&
		+win.SetWindowName(title.c_str())) {
		return true;
	}
	return false;
}

bool Application::InitInput()
{
	if (-gamePads.Create())
		return false;
	if (-gInput.Create(win))
		return false;
	if (-bufferedInput.Create(win))
		return false;
	return true;
}

bool Application::InitAudio()
{
	if (audioEngine.Create() != GReturn::SUCCESS)
	{
		std::cerr << "Failed to initialize audio engine." << std::endl;
		return false;
	}

	return LoadAudioResources();
}

bool Application::LoadAudioResources()
{
	//Lambda to help load and store all of the music / sfx
	auto loadAudio = [&](auto& container, const std::string& key, const std::string& path, float volume)
	{
		decltype(container.begin()->second) audio;
		if (audio.Create(path.c_str(), audioEngine, volume) != GReturn::SUCCESS)
		{
			std::cerr << "Failed to load audio: " << path << std::endl;
			return false;
		}
		container[key] = std::move(audio);
		return true;
	};

	//Load up the music tracks
	if (!loadAudio(musicTracks, "MainMenu", "../Music/Main_Menu.wav", 0.005f) ||
		!loadAudio(musicTracks, "Level1", "../Music/Level_1.wav", 0.005f) ||
		!loadAudio(musicTracks, "Level2", "../Music/Level_2.wav", 0.005f) ||
		!loadAudio(musicTracks, "Level3", "../Music/Level_3.wav", 0.005f))
	{
		return false;
	}

	//Load up the SFX
	if (!loadAudio(soundEffects, "PlayerShoot", "../SoundFX/Player_Attack.wav", 0.2f) ||
		!loadAudio(soundEffects, "PlayerDamage", "../SoundFX/Player_Death.wav", 0.2f) ||
		!loadAudio(soundEffects, "EnemyDeath1", "../SoundFX/Enemy_1_Death.wav", 0.2f) ||
		!loadAudio(soundEffects, "EnemyDeath2", "../SoundFX/Enemy_2_Death.wav", 0.2f) ||
		!loadAudio(soundEffects, "EnemyDeath3", "../SoundFX/Enemy_3_Death.wav", 0.2f) ||
		!loadAudio(soundEffects, "TreasureMetal", "../SoundFX/Treasure_Metal.wav", 0.2f) ||
		!loadAudio(soundEffects, "TreasurePaper", "../SoundFX/Treasure_Paper.wav", 0.2f) ||
		!loadAudio(soundEffects, "UIAccept", "../SoundFX/UI_Menu_Accept.wav", 0.2f) ||
		!loadAudio(soundEffects, "UIScroll", "../SoundFX/UI_Menu_Scroll.wav", 0.2f) ||
		!loadAudio(soundEffects, "UIClick", "../SoundFX/UI_Click.wav", 0.2f) ||
		!loadAudio(soundEffects, "LevelTransition", "../SoundFX/Level_Transition.wav", 0.2f))

	{
		return false;
	}
#ifndef NDEBUG
	std::cout << "All music and sfx loaded!" << std::endl;
#endif
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

bool Application::InitSystems()
{
	// connect systems to global ECS
	if (playerSystem.Init(	game, gameConfig, gInput, bufferedInput,
							gamePads, audioEngine, eventPusher) == false)
		return false;
	if (levelSystem.Init(game, gameConfig, audioEngine) == false)
		return false;
	/*if (vkRenderingSystem.Init(game, gameConfig, vulkan, window) == false)
		return false;*/
	if (physicsSystem.Init(game, gameConfig) == false)
		return false;
	if (bulletSystem.Init(game, gameConfig) == false)
		return false;
	/*if (enemySystem.Init(game, gameConfig, eventPusher) == false)
		return false;*/
	}
	//void Application::AddEntities(Level_Objects& lvl)
	//{
	//	for (auto& i : lvl.allObjectsInLevel)
	//	{
	//		auto e = game->entity(i.name);
	//		e.set<DD::Name>({ i.name });
	//
	//
	//	}
	//	int count = 0;
	//	auto f = game->filter<DD::Name>();
	//
	//	f.each([&count](DD::Name& n)
	//		{
	//			count++;
	//		}
	//	);
	//
	//	std::cout << count << std::endl;
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
