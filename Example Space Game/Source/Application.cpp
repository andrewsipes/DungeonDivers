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
	if (InitEntities() == false)
		return false;
	if (InitSystems() == false)
		return false;
	return true;
}

bool Application::Run() {
	leftMouse = false;
	running = true;

	GEventResponder msgs;
	log.Create("output.txt");

	auto mainMenu = std::make_shared<Level_Objects>();
	auto lvl1 = std::make_shared<Level_Objects>();
	auto currentLevel = std::make_shared<Level_Objects>(); //currentLevel pointer

	float clr[] = { gameConfig->at("BackGroundColor").at("red").as<float>(), gameConfig->at("BackGroundColor").at("blue").as<float>(), gameConfig->at("BackGroundColor").at("green").as<float>(), 1 }; // Buffer

	mainMenu->LoadMeshes(0, "../MainMenu.txt", "../Models/MainMenuModels", log.Relinquish());

	msgs.Create([&](const GW::GEvent& e) {
		GW::SYSTEM::GWindow::Events q;
		if (+e.Read(q) && q == GWindow::Events::RESIZE)
			clr[2] += 0.01f;
		});

	win.Register(msgs);

	if (+ogl.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
		QueryOGLExtensionFunctions(ogl); // Link Needed OpenGL API functions

	gamePlayManager* gpManager;
	PlayerStats* playerStats;
	RendererManager rendererManager(win, ogl, *gameConfig, *this, *mainMenu);

#if NEDEBUG
	auto& mainMenuMusic = musicTracks["MainMenu"];
	mainMenuMusic.Play(true);
#endif


	while (+win.ProcessWindowEvents() && running == true)
	{
		currentLevel->Update(game, currentLevel);

		if (!rendererManager.pauseMenu->render && !rendererManager.isPauseMenuRendered)
			GameLoop();

		glClearColor(clr[0], clr[1], clr[2], clr[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef NDEBUG
		if (rendererManager.mainMenuHUD->render) {
			rendererManager.freecam = false;
		}
#endif


		//Return Left Mouse state for re-use
		if (leftMouse && !(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
			leftMouse = false;
		}

		//event Handling for the mainMenu - starts the game
		else if (rendererManager.mainMenuHUD->render && !rendererManager.controlsMenu->render) {

			if (!leftMouse && rendererManager.mainMenuHUD->startButton->HandleInputLeftMouseButton(gInput)) {
				leftMouse = true;

				lvl1->LoadMeshes(1, "../Models/Level3/GameLevel.txt", "../Models/Level3/Models", log.Relinquish());
				currentLevel = lvl1;

				playerStats = new PlayerStats(*gameConfig);
				gpManager = new gamePlayManager(currentLevel, game);

				gpManager->AddEntities();
				gpManager->AddSystems(currentLevel, game, gameConfig, gInput, bufferedInput, gamePads, audioEngine, eventPusher, playerStats, &rendererManager);

				gpManager->updateEnemyCount(&rendererManager, 0);
				rendererManager.mainMenuHUD->toggleRender();
				rendererManager.playerHUD->toggleRender();
				rendererManager.changeLevel(*currentLevel);
				playerStats->updateHeartsBeforeDeath();
				playerStats->updateScoreBeforeDeath();
			}

		}

		//event handling for pause menu - allows for restart
		else if (rendererManager.pauseMenu->render) {

			if (rendererManager.pauseMenu->restartPauseMenuButton->HandleInputLeftMouseButton(gInput)) {
				leftMouse = true;
				gpManager->restartLevel(currentLevel, &rendererManager, playerStats, log);
				rendererManager.pauseMenu->toggleRender();

			}
		}

		//event handling for pause menu - allows for restart
		else if (rendererManager.gameOverMenu->render) {

			if (rendererManager.gameOverMenu->restartGameOverButton->HandleInputLeftMouseButton(gInput)) {
				leftMouse = true;

				//restarts game by setting current level to level1
				if (rendererManager.gameOverMenu->youWinText->render) {
					gpManager->restartGame(currentLevel, &rendererManager, playerStats, log);
				}

				//restarts just the level per usual
				else {
					gpManager->restartLevel(currentLevel, &rendererManager, playerStats, log);
				}

				rendererManager.gameOverMenu->toggleRender();

			}
		}




			//LEVEL SWAP: Currently works by using 0 or 1
			{
				//use these to determine if flag is read
				bool zero = false, one = false;

				auto lvl3 = std::make_shared<Level_Objects>();

				//Main Menu
				if (!zero && (GetAsyncKeyState(0x30) & 0x8000)) {
					zero = true;

					//rendererManager.changeLevel(*mainMenu);
					//gpManager.restartLevel(currentLevel, &rendererManager, &playerStats, log);

					rendererManager.gameOverMenu->youWinText->render =false;
					rendererManager.gameOverMenu->gameOverText->render = true;
					rendererManager.gameOverMenu->toggleRender();


				}

				else if (zero && !(GetAsyncKeyState(0x30) & 0x8000)) {
					zero = false;
				}

				//Level1
				if (!one && (GetAsyncKeyState(0x31) & 0x8000)) {
					one = true;

					rendererManager.changeLevel(*lvl1);

				}

				else if (zero && !(GetAsyncKeyState(0x30) & 0x8000)) {
					one = false;
				}
			}


			rendererManager.UpdateCamera(gameConfig->at("Window").at("width").as<int>(), gameConfig->at("Window").at("height").as<int>());
			rendererManager.Render();
			ogl.UniversalSwapBuffers();

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
	if (physicsSystem.Init(game, gameConfig) == false)
		return false;
	if (bulletSystem.Init(game, gameConfig) == false)
		return false;
	/*if (enemySystem.Init(game, gameConfig, eventPusher) == false)
		return false;*/
	}

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
