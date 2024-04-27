#include "PhysicsLogic.h"
#include "../Components/Physics.h"
#include "../Components/Identification.h"
#include "../Components/Gameplay.h"

bool ESG::PhysicsLogic::Init(std::shared_ptr<flecs::world> _game, std::weak_ptr<const GameConfig> _gameConfig)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;

	// **** MOVEMENT ****
	// update velocity by acceleration
	game->system<Velocity, const Acceleration>("Acceleration System")
		.each([](flecs::entity e, Velocity& v, const Acceleration& a)
			{
				GW::MATH2D::GVECTOR2F accel;
				GW::MATH2D::GVector2D::Scale2F(a.value, e.delta_time(), accel);
				GW::MATH2D::GVector2D::Add2F(accel, v.value, v.value);
			});

	// update position by velocity
	game->system<Position, const Velocity>("Translation System")
		.each([](flecs::entity e, Position& p, const Velocity& v)
			{
				GW::MATH2D::GVECTOR2F speed;
				GW::MATH2D::GVector2D::Scale2F(v.value, e.delta_time(), speed);

				// adding is simple but doesn't account for orientation
				GW::MATH2D::GVector2D::Add2F(speed, p.value, p.value);
			});

	// **** CLEANUP ****
	// clean up any objects that end up offscreen
	game->system<const Position>("Cleanup System")
		.each([](flecs::entity e, const Position& p)
			{
				if (p.value.x > 1.5f || p.value.x < -1.5f || p.value.y > 1.5f || p.value.y < -1.5f)
				{
					e.destruct();
				}
			});

	// **** COLLISIONS ****
	// due to wanting to loop through all collidables at once, we do this in two steps:
	// 1. A System will gather all collidables into a shared std::vector
	// 2. A second system will run after, testing/resolving all collidables against each other
	queryCache = game->query<Collidable, World>();

	// only happens once per frame at the very start of the frame
	struct CollisionSystem {}; // local definition so we control iteration count (singular)
	game->entity("Detect-Collisions").add<CollisionSystem>();

	game->system<CollisionSystem>()
		.each([this](CollisionSystem& s)
			{
				//std::cout << queryCache.count() << std::endl;
				
				// collect any and all collidable objects
				queryCache.each([&](flecs::entity e, Collidable& c, World& w)
					{
						GW::MATH::GMATRIXF matrix = w.value;

						SHAPE box; // compute buffer for this objects polygon

						// This is critical, if you want to store an entity handle it must be mutable
						box.owner = e; // allows later changes
						box.obby = c.obb;
						box.obby.center = w.value.row4;
						box.name = e.get<ESG::Name>()->name;

						// add to vector
						testCache.push_back(box);
					});

				// loop through the testCahe resolving all collisions
				for (int i = 0; i < testCache.size(); ++i)
				{
					// the inner loop starts at the entity after you so you don't double check collisions
					for (int j = i + 1; j < testCache.size(); ++j)
					{
						// test the two world space polygons for collision
						// possibly make this cheaper by leaving one of them local and using an inverse matrix
						GW::MATH::GCollision::GCollisionCheck results;
						GW::MATH::GCollision::TestOBBToOBBF(testCache[i].obby, testCache[j].obby, results);

						if (results == GW::MATH::GCollision::GCollisionCheck::COLLISION)
						{
							testCache[i].owner.add<CollidedWith>(testCache[j].owner);
							testCache[j].owner.add<CollidedWith>(testCache[i].owner);
							
							#ifndef NDEBUG
							std::cout << "Collision Detected between:  1: " << testCache[i].owner.get<Name>()->name << "    2. " << testCache[j].owner.get<Name>()->name << std::endl;
							#endif
						}
					}
				}
				// wipe the test cache for the next frame (keeps capacity intact)
				testCache.clear();
			});

	return true;
}

bool ESG::PhysicsLogic::Activate(bool runSystem)
{
	if (runSystem)
	{
		game->entity("Acceleration System").enable();
		game->entity("Translation System").enable();
		game->entity("Cleanup System").enable();
	}
	else
	{
		game->entity("Acceleration System").disable();
		game->entity("Translation System").disable();
		game->entity("Cleanup System").disable();
	}
	return true;
}

bool ESG::PhysicsLogic::Shutdown()
{
	queryCache.destruct(); // fixes crash on shutdown
	game->entity("Acceleration System").destruct();
	game->entity("Translation System").destruct();
	game->entity("Cleanup System").destruct();
	return true;
}