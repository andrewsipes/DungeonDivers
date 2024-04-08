#include "PlayerData.h"
#include "../Components/Identification.h"
#include "../Components/Visuals.h"
#include "../Components/Physics.h"
#include "Prefabs.h"

bool ESG::PlayerData::Load(  std::shared_ptr<flecs::world> _game, 
                            std::weak_ptr<const GameConfig> _gameConfig)
{
	// Grab init settings for players
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();
	// color
	float red = (*readCfg).at("Player1").at("red").as<float>();
	float green = (*readCfg).at("Player1").at("green").as<float>();
	float blue = (*readCfg).at("Player1").at("blue").as<float>();
	// start position
	float xstart = (*readCfg).at("Player1").at("xstart").as<float>();
	float ystart = (*readCfg).at("Player1").at("ystart").as<float>();
	float scale = (*readCfg).at("Player1").at("scale").as<float>();

	// Create Player One
	_game->entity("Player One")
	.set([&](Position& p, Orientation& o, Material& m, ControllerID& c) {
		c = { 0 };
		p = { xstart, ystart };
		m = { red, green, blue };
		o = { GW::MATH2D::GIdentityMatrix2F };
		GW::MATH2D::GMatrix2D::Scale2F(o.value, GW::MATH2D::GVECTOR2F{ scale, scale }, o.value);
	})
	.add<Player>(); // Tag this entity as a Player

	return true;
}

bool ESG::PlayerData::Unload(std::shared_ptr<flecs::world> _game)
{
	// remove all players
	_game->defer_begin(); // required when removing while iterating!
		_game->each([](flecs::entity e, Player&) {
			e.destruct(); // destroy this entitiy (happens at frame end)
		});
	_game->defer_end(); // required when removing while iterating!

    return true;
}
