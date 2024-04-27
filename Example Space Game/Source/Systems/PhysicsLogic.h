// The level system is responsible for transitioning the various levels in the game
#ifndef PHYSICSLOGIC_H
#define PHYSICSLOGIC_H

// Contains our global game settings
#include "../GameConfig.h"
#include "../Components/Physics.h"

// Dungeon Divers (avoid name collisions)
namespace DD
{
	class PhysicsLogic
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;

		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;

		// used to cache collision queries
		flecs::query<Collidable, World> queryCache;

		// defines what to be tested
		static constexpr unsigned boxSize = 4;

		struct SHAPE 
		{
			GW::MATH::GOBBF obby;
			flecs::entity owner;
			std::string name;
			// Modify the polygon array to store 3D points
			//GW::MATH::GVECTORF boundingBox[boxSize];
		};

		// vector used to save/cache all active collidables
		std::vector<SHAPE> testCache;

	public:
		// attach the required logic to the ECS 
		bool Init(std::shared_ptr<flecs::world> _game, std::weak_ptr<const GameConfig> _gameConfig);

		// control if the system is actively running
		bool Activate(bool runSystem);

		// release any resources allocated by the system
		bool Shutdown();
	};
};

#endif