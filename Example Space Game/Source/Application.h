#ifndef APPLICATION_H
#define APPLICATION_H

#include <map>
#include <string>
// include events
#include "Events/Playevents.h"
// Contains our global game settings
#include "GameConfig.h"
// Load all entities+prefabs used by the game
#include "Entities/BulletData.h"
#include "Entities/PlayerData.h"
#include "Entities/EnemyData.h"

// Include all systems used by the game and their associated components
#include "Systems/PlayerLogic.h"
#include "Systems/VulkanRendererLogic.h"
#include "Systems/LevelLogic.h"
#include "Systems/PhysicsLogic.h"
#include "Systems/BulletLogic.h"
#include "Systems/EnemyLogic.h"
#include "Components/Identification.h"

//include PlayerStats
#include "playerStats.h"

// Allocates and runs all sub-systems essential to operating the game
class Application
{
	// gateware libs used to access operating system
	GW::SYSTEM::GWindow win; // gateware multi-platform window

	GW::GRAPHICS::GOpenGLSurface ogl; // gateware openGL API wrapper
	GW::INPUT::GController gamePads; // controller support
	GW::INPUT::GInput gInput; // twitch keybaord/mouse
	GW::INPUT::GBufferedInput bufferedInput; // event keyboard/mouse
	GW::AUDIO::GAudio audioEngine; // can create music & sound effects
	std::map<std::string, GW::AUDIO::GMusic> musicTracks;
	std::map<std::string, GW::AUDIO::GSound> soundEffects;
	GW::AUDIO::GMusic currentTrack;
	// third-party gameplay & utility libraries
	std::shared_ptr<flecs::world> game; // ECS database for gameplay
	//std::shared_ptr<Level_Objects> lvl;
	std::shared_ptr<GameConfig> gameConfig; // .ini file game settings

	// ECS Entities and Prefabs that need to be loaded
	DD::BulletData weapons;
	DD::PlayerData players;
	DD::EnemyData enemies;
	// specific ECS systems used to run the game
	DD::PlayerLogic playerSystem;
	//DD::VulkanRendererLogic vkRenderingSystem;
	DD::LevelLogic levelSystem;
	DD::PhysicsLogic physicsSystem;
	DD::BulletLogic bulletSystem;
	DD::EnemyLogic enemySystem;
	// EventGenerator for Game Events
	GW::CORE::GEventGenerator eventPusher;

	bool leftMouse;

public:
	GW::SYSTEM::GLog log;
	bool running;
	bool Init();
	bool Run();
	bool Shutdown();
private:

	bool InitWindow();
	bool InitInput();
	bool InitAudio();
	bool InitEntities();
	bool InitSystems();
	bool GameLoop();

	//separate audio manager from initaudio
	bool LoadAudioResources();
};

#endif 