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

loadingUi* load;
int currentLevelNumber = 0;
std::map<std::string, GW::AUDIO::GSound> musicTracks;

bool Application::Init()
{
	//eventPusher.Create();
	//	GW::AUDIO::GSound shoot;
	//				GW::GReturn test = shoot.Create("../SoundFX/UI_Click.wav", audioEngine, 1.0f);
	//	GW::AUDIO::GSound shoot;
	//				GW::GReturn test = shoot.Create("../SoundFX/UI_Click.wav", audioEngine, 1.0f);


	// load all game settigns
	gameConfig = std::make_shared<GameConfig>();
	// create the ECS system
	game = std::make_shared<flecs::world>();

	load = new loadingUi(*gameConfig);
	load->LoadMeshes("../load.txt", "../Models/loadScreen", log.Relinquish());
	load->assign();
	load->arrange();
	load->start();

	

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
	GEventResponder msgs;
	
	float clr[] = { gameConfig->at("BackGroundColor").at("red").as<float>(), gameConfig->at("BackGroundColor").at("blue").as<float>(), gameConfig->at("BackGroundColor").at("green").as<float>(), 1 }; // Buffer
	
	msgs.Create([&](const GW::GEvent& e) {
		GW::SYSTEM::GWindow::Events q;
		if (+e.Read(q) && q == GWindow::Events::RESIZE)
			clr[2] += 0.01f;
		});

	win.Register(msgs);

	if (+ogl.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
		QueryOGLExtensionFunctions(ogl); // Link Needed OpenGL API functions

	ogl.UniversalSwapBuffers();

	RendererManager rendererManager(win, ogl, *gameConfig, *this, load);

	leftMouse = false;
	running = true;
	levelComplete = false;

	log.Create("output.txt");

	auto currentLevel = std::make_shared<Level_Objects>(); //currentLevel pointer

	currentLevel->LoadMeshes(1, "../Level1.txt", "../Models/Level1", log.Relinquish());
	gamePlayManager gpManager(currentLevel, game);
	PlayerStats playerStats(*gameConfig);

	gpManager.AddEntities();
	gpManager.AddSystems(currentLevel, game, gameConfig, gInput, bufferedInput, gamePads, audioEngine, eventPusher, &playerStats, &rendererManager);

	auto& mainMenuMusic = musicTracks["MainMenu"];
	mainMenuMusic.Play(true);


	while (+win.ProcessWindowEvents() && running == true)
	{
		currentLevel->Update(game, currentLevel);
		currentLevel->postProcessing();

		if (!rendererManager.pauseMenu->render && !rendererManager.isPauseMenuRendered && !rendererManager.gameOverMenu->render)
			GameLoop();

		glClearColor(clr[0], clr[1], clr[2], clr[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef NDEBUG
		if (rendererManager.mainMenuHUD->render) {
			rendererManager.freecam = false;
		}
#endif
		if (levelComplete && !(GetAsyncKeyState(VK_SPACE) & 0x8000)) {

			levelComplete = false;

		}

		//Return Left Mouse state for re-use
		if (leftMouse && !(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
			leftMouse = false;
		}


		//event Handling for the mainMenu - starts the game
		else if (rendererManager.mainMenuHUD->render && !rendererManager.controlsMenu->render) {

			if (!leftMouse && rendererManager.mainMenuHUD->startButton->HandleInputLeftMouseButton(gInput)) {
				leftMouse = true;

				
				//Attempting to stop the main menu music and start level 1 music, but only the main menu stops
				gpManager.updateTreasureCount(&rendererManager, 0);
				rendererManager.mainMenuHUD->toggleRender();
				rendererManager.playerHUD->toggleRender();
				mainMenuMusic.Stop();
				rendererManager.changeLevel(currentLevel);
				currentLevelNumber = 1;
				playerStats.updateHeartsBeforeDeath();
				playerStats.updateScoreBeforeDeath();
				rendererManager.playerHUD->startText->render = true;
			}

		}

		//event handling for pause menu - allows for restart
		else if (rendererManager.pauseMenu->render) {

			if (rendererManager.pauseMenu->restartPauseMenuButton->HandleInputLeftMouseButton(gInput)) {
				leftMouse = true;
				gpManager.restartLevel(currentLevel, &rendererManager, &playerStats, log);
				rendererManager.pauseMenu->toggleRender();

			}
		}

		//event handling for pause menu - allows for restart
		else if (rendererManager.gameOverMenu->render) {

			if (rendererManager.gameOverMenu->restartGameOverButton->HandleInputLeftMouseButton(gInput)) {
				leftMouse = true;

				//restarts game by setting current level to level1
				if (rendererManager.gameOverMenu->youWinText->render) {
					rendererManager.playerHUD->continueText->toggleRender();
					rendererManager.playerHUD->levelCompleteText->toggleRender();
					rendererManager.playerHUD->toggleRender();
					gpManager.restartLevel(currentLevel, &rendererManager, &playerStats, log);
					gpManager.AddSystems(currentLevel, game, gameConfig, gInput, bufferedInput, gamePads, audioEngine, eventPusher, &playerStats, &rendererManager);
				}

				//restarts just the level per usual
				else {
					gpManager.restartLevel(currentLevel, &rendererManager, &playerStats, log);
				}

				rendererManager.gameOverMenu->toggleRender();

			}
		}

		if (gpManager.getTreasuresInLevel() <= 0) {

			rendererManager.playerHUD->levelCompleteText->render = true;
			rendererManager.playerHUD->continueText->render = true;


			if (!levelComplete && (GetAsyncKeyState(VK_SPACE) & 0x8000)) {
				levelComplete = true;

			}

		}

		if (levelComplete && !rendererManager.gameOverMenu->render) {

			switch (playerStats.treasures) {

			case 3:
				gpManager.nextLevel(currentLevel, &playerStats, &rendererManager, log);
				gpManager.AddSystems(currentLevel, game, gameConfig, gInput, bufferedInput, gamePads, audioEngine, eventPusher, &playerStats, &rendererManager);		
				currentLevelNumber = 2;
				break;
			case 6:
				gpManager.nextLevel(currentLevel, &playerStats, &rendererManager, log);
				gpManager.AddSystems(currentLevel, game, gameConfig, gInput, bufferedInput, gamePads, audioEngine, eventPusher, &playerStats, &rendererManager);
				currentLevelNumber = 3;
				break;
			case 9:
				gpManager.RemoveEntities();
				rendererManager.playerHUD->toggleRender();
				rendererManager.gameOverMenu->youWin(playerStats.getScore(), gameConfig->at("Player1").at("highscore").as<int>());
				rendererManager.gameOverMenu->toggleRender();
				break;
			default:
				//do nothing
				break;
			}
			//Music counter which works with the LoadAudio musicTrack container
			std::string levelMusic = "Level" + std::to_string(currentLevelNumber);
			auto musicIter = musicTracks.find(levelMusic);
			if (musicIter != musicTracks.end()) {
				auto& levelMusicTrack = musicIter->second;
				levelMusicTrack.Play(true);
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

	////Load up the SFX
	//if (!loadAudio(soundEffects, "PlayerShoot", "../SoundFX/Player_Attack.wav", 0.2f) ||
	//	!loadAudio(soundEffects, "PlayerDamage", "../SoundFX/Player_Death.wav", 0.2f) ||
	//	!loadAudio(soundEffects, "EnemyDeath1", "../SoundFX/Enemy_1_Death.wav", 0.2f) ||
	//	!loadAudio(soundEffects, "EnemyDeath2", "../SoundFX/Enemy_2_Death.wav", 0.2f) ||
	//	!loadAudio(soundEffects, "EnemyDeath3", "../SoundFX/Enemy_3_Death.wav", 0.2f) ||
	//	!loadAudio(soundEffects, "TreasureMetal", "../SoundFX/Treasure_Metal.wav", 0.2f) ||
	//	!loadAudio(soundEffects, "TreasurePaper", "../SoundFX/Treasure_Paper.wav", 0.2f) ||
	//	!loadAudio(soundEffects, "UIAccept", "../SoundFX/UI_Menu_Accept.wav", 0.2f) ||
	//	!loadAudio(soundEffects, "UIScroll", "../SoundFX/UI_Menu_Scroll.wav", 0.2f) ||
	//	!loadAudio(soundEffects, "UIClick", "../SoundFX/UI_Click.wav", 0.2f) ||
	//	!loadAudio(soundEffects, "LevelTransition", "../SoundFX/Level_Transition.wav", 0.2f))

	//{
	//	return false;
	//}
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
