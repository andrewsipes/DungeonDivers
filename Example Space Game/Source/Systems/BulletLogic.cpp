#include <random>
#include "BulletLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"

using namespace DD; // Dungeon Divers

// Connects logic to traverse any players and allow a controller to manipulate them
bool DD::BulletLogic::Init(std::shared_ptr<flecs::world> _game, std::weak_ptr<const GameConfig> _gameConfig)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;

	// destroy any bullets that have the CollidedWith relationship
	//game->system<Bullet>("Bullet System")
	//	.each([](flecs::entity arrow, Bullet) 
	//{
	//	// damage anything we come into contact with
	//	arrow.each<CollidedWith>([&arrow](flecs::entity hit)
	//	{
	//		if (hit.has<Health>()) 
	//		{
	//			// reduce the health of the hit entity by the damage value
	//			int current = hit.get<Health>()->value;

	//		}	
	//	});

	//	// if you have collidedWith relationship then be destroyed
	//	if (arrow.has<CollidedWith>(flecs::Wildcard))
	//	{
	//		arrow.destruct();
	//	}
	//});
	
	return true;
}

// Free any resources used to run this system
bool DD::BulletLogic::Shutdown()
{
	game->entity("Bullet System").destruct();

	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

// Toggle if a system's Logic is actively running
bool DD::BulletLogic::Activate(bool runSystem)
{
	if (runSystem) 
	{
		game->entity("Bullet System").enable();
	}
	else
	{
		game->entity("Bullet System").disable();
	}
	return false;
}
