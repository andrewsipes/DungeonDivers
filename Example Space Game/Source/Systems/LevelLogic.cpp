#include <random>
#include "LevelLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Entities/Prefabs.h"
#include "../Utils/Macros.h"
#include "LevelLogic.h"

using namespace ESG; // Example Space Game

// Connects logic to traverse any players and allow a controller to manipulate them
bool ESG::LevelLogic::Init(std::shared_ptr<flecs::world> _game, std::weak_ptr<const GameConfig> _gameConfig, GW::AUDIO::GAudio _audioEngine)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	audioEngine = _audioEngine;

	// create an asynchronous version of the world
	gameAsync = game->async_stage(); // just used for adding stuff, don't try to read data
	gameLock.Create();

	// Pull enemy Y start location from config file
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();
	float enemy1startY = (*readCfg).at("Enemy1").at("ystart").as<float>();
	float enemy1accmax = (*readCfg).at("Enemy1").at("accmax").as<float>();
	float enemy1accmin = (*readCfg).at("Enemy1").at("accmin").as<float>();

	// level one info
	float spawnDelay = (*readCfg).at("Level1").at("spawndelay").as<float>();
	
	// spins up a job in a thread pool to invoke a function at a regular interval
	timedEvents.Create(spawnDelay * 1000, [this, enemy1startY, enemy1accmax, enemy1accmin]() 
	{
		// compute random spawn location
		std::random_device rd;  // Will be used to obtain a seed for the random number engine
		std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
		std::uniform_real_distribution<float> x_range(-0.9f, +0.9f);
		std::uniform_real_distribution<float> a_range(enemy1accmin, enemy1accmax);
		float Xstart = x_range(gen); // normal rand() doesn't work great multi-threaded
		float accel = a_range(gen);

		// grab enemy type 1 prefab
		flecs::entity et1; 

		if (RetreivePrefab("Enemy Type1", et1)) 
		{
			// you must ensure the async_stage is thread safe as it has no built-in synchronization
			gameLock.LockSyncWrite();

			// this method of using prefabs is pretty conveinent
			gameAsync.entity().is_a(et1)
				.set<Velocity>({ 0,0 })
				.set<Acceleration>({ 0, -accel })
				.set<Position>({ Xstart, enemy1startY });

			// be sure to unlock when done so the main thread can safely merge the changes
			gameLock.UnlockSyncWrite();
		}
	}, 5000); // wait 5 seconds to start enemy wave

	// create a system the runs at the end of the frame only once to merge async changes
	struct LevelSystem {}; // local definition so we control iteration counts
	game->entity("Level System").add<LevelSystem>();

	// only happens once per frame at the very start of the frame
	game->system<LevelSystem>().kind(flecs::OnLoad) // first defined phase
		.each([this](flecs::entity e, LevelSystem& s) 
	{
		// merge any waiting changes from the last frame that happened on other threads
		gameLock.LockSyncWrite();
		gameAsync.merge();
		gameLock.UnlockSyncWrite();
	});

	// Load and play level one's music
	currentTrack.Create("../Music/Space Ambience.wav", audioEngine, 0.15f);
	currentTrack.Play(false);

	return true;
}

// Implementation of DetectCollisions() function
void ESG::LevelLogic::DetectCollisions() 
{
	CheckPlayerBulletCollisions();
	CheckEnemyBulletCollisions();
	CheckPlayerEnemyCollisions();
	CheckPlayerEnvironmentCollisions();
}

// Free any resources used to run this system
bool ESG::LevelLogic::Shutdown()
{
	timedEvents = nullptr; // stop adding enemies
	gameAsync.merge(); // get rid of any remaining commands
	game->entity("Level System").destruct();

	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

// Toggle if a system's Logic is actively running
bool ESG::LevelLogic::Activate(bool runSystem)
{
	if (runSystem) 
	{
		game->entity("Level System").enable();
	}
	else 
	{
		game->entity("Level System").disable();
	}
	return false;
}

void ESG::LevelLogic::CheckPlayerBulletCollisions() 
{
	// Loop through all players
	//for (auto& player : game->query<PlayerData>().entities()) 
	//{
	//	// Get the player's position and size
	//	Position& playerPos = player.get<Position>();
	//	float playerSize = player.get<Size>().value;
	//	// Loop through all bullets
	//	for (auto& bullet : game->query<BulletData>().entities()) 
	//	{
	//		// Get the bullet's position and size
	//		Position& bulletPos = bullet.get<Position>();
	//		float bulletSize = bullet.get<Size>().value;
	//		// Calculate the distance between the player and the bullet
	//		float distance = sqrt(pow(playerPos.x - bulletPos.x, 2) + pow(playerPos.y - bulletPos.y, 2));
	//		// Check if the distance is less than the sum of their radii (they intersect)
	//		if (distance < (playerSize + bulletSize) / 2) 
	//		{
	//			// Collision detected!
	//			// Add your collision handling logic here
	//			// Decrease player's health
	//			player.get<Health>().value -= bullet.get<Damage>().value;
	//			// Remove the bullet entity from the game
	//			game->delete_entity(bullet);
	//			// Check if the player's health has dropped to zero or below
	//			if (player.get<Health>().value <= 0) 
	//			{
	//				// Player has been destroyed
	//				// Perform any additional actions, such as respawning the player, ending the game, etc.
	//			}
	//		}
	//	}
	//}
}

void ESG::LevelLogic::CheckEnemyBulletCollisions() 
{
	// Implement collision detection logic between enemies and bullets
}

void ESG::LevelLogic::CheckPlayerEnemyCollisions() 
{
	// Implement collision detection logic between players and enemies
}

void ESG::LevelLogic::CheckPlayerEnvironmentCollisions()
{
	//// Loop through all players in the game
	//for (auto player : game->entities().view<PlayerData>()) 
	//{
	//	// Get the position and size of the player
	//	Position& playerPosition = player.get<Position>();
	//	float playerSize = player.get<Size>().value;
	//	// Check collision with each environment object (e.g., walls, obstacles)
	//	for (auto envObj : game->entities().view<EnvironmentObject>()) 
	//	{
	//		// Get the position and size of the environment object
	//		Position& envObjPosition = envObj.get<Position>();
	//		float envObjSize = envObj.get<Size>().value;
	//		// Calculate the distance between the player and the environment object
	//		float distance = calculateDistance(playerPosition, envObjPosition);
	//		// Check if the distance is less than the sum of their radii (they intersect)
	//		if (distance < (playerSize + envObjSize) / 2) 
	//		{
	//			// Collision detected!
	//		}
	//	}
	//}
}


// Implementation of Init() function
bool ESG::LevelLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::AUDIO::GAudio _audioEngine) {
	// Initialize game, gameConfig, audioEngine, and other members
}

// **** SAMPLE OF MULTI_THREADED USE ****
//flecs::world world; // main world
//flecs::world async_stage = world.async_stage();
//
//// From thread
//lock(async_stage_lock);
//flecs::entity e = async_stage.entity().child_of(parent)...
//unlock(async_stage_lock);
//
//// From main thread, periodic
//lock(async_stage_lock);
//async_stage.merge(); // merge all commands to main world
//unlock(async_stage_lock);