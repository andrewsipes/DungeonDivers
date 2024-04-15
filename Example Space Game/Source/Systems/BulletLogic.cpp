#include <random>
#include "BulletLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"

using namespace ESG; // Example Space Game

// Connects logic to traverse any players and allow a controller to manipulate them
bool ESG::BulletLogic::Init(std::shared_ptr<flecs::world> _game, std::weak_ptr<const GameConfig> _gameConfig)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;

	// destroy any bullets that have the CollidedWith relationship
	game->system<Bullet, Damage>("Bullet System")
		.each([](flecs::entity arrow, Bullet, Damage &d) 
	{
		// damage anything we come into contact with
		arrow.each<CollidedWith>([&arrow, d](flecs::entity hit)
		{
			if (hit.has<Health>()) 
			{
				// reduce the health of the hit entity by the damage value
				int current = hit.get<Health>()->value;
				hit.set<Health>({ current - d.value });

				// reduce the amount of hits but the charged shot
				if (arrow.has<ChargedShot>() && hit.get<Health>()->value <= 0)
				{
					int md_count = arrow.get<ChargedShot>()->max_destroy;
					arrow.set<ChargedShot>({ md_count - 1 });
				}
			}	
		});
		// if you have collidedWith relationship then be destroyed
		if (arrow.has<CollidedWith>(flecs::Wildcard))
		{
			// Destroy the arrow entity if it has a 'ChargedShot' component and 
				// its 'max_destroy' value is zero or less
			if (arrow.has<ChargedShot>())
			{
				// ChargedShot is number of times a bullet can hit target before 
					// its destroyed
				if(arrow.get<ChargedShot>()->max_destroy <= 0)
					arrow.destruct();
			}
			else 
			{
				// play hit sound
				arrow.destruct();
			}
		}
	});
	
	return true;
}

// Free any resources used to run this system
bool ESG::BulletLogic::Shutdown()
{
	game->entity("Bullet System").destruct();

	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

// Toggle if a system's Logic is actively running
bool ESG::BulletLogic::Activate(bool runSystem)
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
