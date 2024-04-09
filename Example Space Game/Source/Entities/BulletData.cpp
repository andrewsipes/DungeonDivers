#include <memory>
#include "BulletData.h"
#include "../Components/Identification.h"
#include "../Components/Visuals.h"
#include "../Components/Physics.h"
#include "Prefabs.h"
#include "../Components/Gameplay.h"

bool ESG::BulletData::Load(std::shared_ptr<flecs::world> _game, std::weak_ptr<const GameConfig> _gameConfig, GW::AUDIO::GAudio _audioEngine)
{
	// Grab init settings for players
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();
	
	// Create prefab for laser weapon
	// color
	float red = (*readCfg).at("Lazers").at("red").as<float>();
	float green = (*readCfg).at("Lazers").at("green").as<float>();
	float blue = (*readCfg).at("Lazers").at("blue").as<float>();

	// other attributes
	float speed = (*readCfg).at("Lazers").at("speed").as<float>();
	float xscale = (*readCfg).at("Lazers").at("xscale").as<float>();
	float yscale = (*readCfg).at("Lazers").at("yscale").as<float>();
	int dmg = (*readCfg).at("Lazers").at("damage").as<int>();
	int pcount = (*readCfg).at("Lazers").at("projectiles").as<int>();
	float frate = (*readCfg).at("Lazers").at("firerate").as<float>();
	std::string fireFX = (*readCfg).at("Lazers").at("fireFX").as<std::string>();

	// default projectile scale
	GW::MATH2D::GMATRIX2F world;
	GW::MATH2D::GMatrix2D::Scale2F(GW::MATH2D::GIdentityMatrix2F, 
	GW::MATH2D::GVECTOR2F{ xscale, yscale }, world);

	// Load sound effect used by this bullet prefab
	GW::AUDIO::GSound shoot;
	shoot.Create(fireFX.c_str(), _audioEngine, 0.15f); // we need a global music & sfx volumes

	// add prefab to ECS
	auto lazerPrefab = _game->prefab()
		// .set<> in a prefab means components are shared (instanced)
		.set<Material>({ red, green, blue })
		.set<Orientation>({ world })
		.set<Acceleration>({ 0, 0 })
		.set<Velocity>({ 0, speed })
		.set<GW::AUDIO::GSound>(shoot.Relinquish())

		// .override<> ensures a component is unique to each entity created from a prefab 
		.set_override<Damage>({ dmg })
		//.set_override<ChargedShot>({ 2 })
		.override<Position>()
		.override<Bullet>() // Tag this prefab as a bullet (for queries/systems)
		.override<Collidable>(); // can be collided with

	// register this prefab by name so other systems can use it
	RegisterPrefab("Lazer Bullet", lazerPrefab);

	return true;
}

bool ESG::BulletData::Unload(std::shared_ptr<flecs::world> _game)
{
	// remove all bullets and their prefabs
	_game->defer_begin(); // required when removing while iterating!
	_game->each([](flecs::entity e, Bullet&) 
	{
		e.destruct(); // destroy this entitiy (happens at frame end)
	});
	_game->defer_end(); // required when removing while iterating!

	// unregister this prefab by name
	UnregisterPrefab("Lazer Bullet");

	return true;
}
