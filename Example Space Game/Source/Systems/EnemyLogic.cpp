#include <random>
#include "EnemyLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"
#include "../Events/Playevents.h"

using namespace ESG; // Example Space Game
using namespace GW::INPUT; // input libs
using namespace GW::AUDIO; // audio libs

// Connects logic to traverse any players and allow a controller to manipulate them
bool ESG::EnemyLogic::Init(std::shared_ptr<flecs::world> _game, std::weak_ptr<const GameConfig> _gameConfig,
	GW::AUDIO::GAudio _audioEngine, GW::CORE::GEventGenerator _eventPusher)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	audioEngine = _audioEngine;

	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();

	int width = (*readCfg).at("Window").at("width").as<int>();
	float speed = (*readCfg).at("Enemy1").at("speed").as<float>();

	/*collisionSystem = game->system<Position, Collidable>("Collision System")
		.iter([&](flecs::entity e, ESG::Position& pos, ESG::Collidable& collided)
			{
				auto beeEntity = game->entity("Bee");
				if (beeEntity.is_alive()) 
				{
					beeEntity.set<Collidable>({ GW::MATH2D::GRECTANGLE2D{ });
					std::cout << "Collision box added to enemy" << std::endl;
				}
				else 
				{
					std::cout << "Unable to find enemy entity named Bee" << std::endl;
				}
			});*/

	// destroy any bullets that have the CollidedWith relationship
	game->system<Enemy, Health>("Enemy System")
		.each([this, speed](flecs::entity e, Enemy, Health& h)
	{
			// if you have no health left be destroyed
			if (e.get<Health>()->value <= 0) 
			{
				// play explode sound
				e.destruct();
				ESG::PLAY_EVENT_DATA x;
				GW::GEvent explode;
				explode.Write(ESG::PLAY_EVENT::ENEMY_DESTROYED, x);
 				eventPusher.Push(explode);
			} 		
	});

	return true;
}

// Free any resources used to run this system
bool ESG::EnemyLogic::Shutdown()
{
	game->entity("Enemy System").destruct();
	//enemySystem.destruct();

	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();

	return true;
}

// Toggle if a system's Logic is actively running
bool ESG::EnemyLogic::Activate(bool runSystem)
{
	if (runSystem) 
	{
		game->entity("Enemy System").enable();
	}
	else 
	{
		game->entity("Enemy System").disable();
	}
	return false;

	//if (enemySystem.is_alive())
	//{
	//	(runSystem) ?
	//		enemySystem.enable()
	//		: enemySystem.disable();
	//	return true;
	//}
	//return false;

	/*if (collisionSystem.is_alive()) {
		(runSystem) ? collisionSystem.enable() : collisionSystem.disable();
		return true;
	}
	return false;*/
}