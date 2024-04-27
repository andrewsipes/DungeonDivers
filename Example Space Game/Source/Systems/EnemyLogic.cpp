#include <random>
#include "EnemyLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"
#include "../Events/Playevents.h"
//#include "../Systems/PhysicsLogic.h"

using namespace DD; // Dungeon Divers
using namespace GW::INPUT; // input libs
using namespace GW::AUDIO; // audio libs

// Connects logic to traverse any players and allow a controller to manipulate them
bool DD::EnemyLogic::Init(std::shared_ptr<flecs::world> _game, std::weak_ptr<const GameConfig> _gameConfig, GW::AUDIO::GAudio _audioEngine, GW::CORE::GEventGenerator _eventPusher)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	audioEngine = _audioEngine;

	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();

	int width = (*readCfg).at("Window").at("width").as<int>();
	float speed = (*readCfg).at("Enemy1").at("speed").as<float>();

	//flecs::system enemySystem = game->system<ESG::Enemy, ESG::World>("Enemy Movement System")
	//		.iter([this, speed](flecs::iter it, ESG::Enemy*, ESG::World* w)
	//			{
	//				for (auto i : it)
	//				{
	//					// get player position
	//					float xaxis = 1, zaxis = 0;
	//
	//					GW::MATH::GVECTORF v = { xaxis * it.delta_time() * speed, 0, zaxis * it.delta_time() * speed };
	//					auto ene = game->lookup("alien");
	//					ene.set<ESG::LastWorld>({ ene.get<ESG::World>()->value });
	//					ESG::World* edit = game->entity(ene).get_mut<ESG::World>();
	//
	//					GW::MATH::GMatrix::TranslateLocalF(edit->value, v, edit->value);
	//
	//				}
	//			});

	// destroy any bullets that have the CollidedWith relationship

	//game->system<Enemy, Health>("Enemy System")
	//	.each([this, speed](flecs::entity e, Enemy, Health& h)
	//		{
	//			// if you have no health left be destroyed
	//			if (e.get<Health>()->value <= 0)
	//			{
	//				// play explode sound
	//				e.destruct();
	//				DD::PLAY_EVENT_DATA x;
	//				GW::GEvent explode;
	//				explode.Write(DD::PLAY_EVENT::ENEMY_DESTROYED, x);
	//				eventPusher.Push(explode);
	//			}
	//		});

	return true;
}

// Free any resources used to run this system
bool DD::EnemyLogic::Shutdown()
{
	game->entity("Enemy System").destruct();
	//enemySystem.destruct();

	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();

	return true;
}

// Toggle if a system's Logic is actively running
bool DD::EnemyLogic::Activate(bool runSystem)
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