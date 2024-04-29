// define all ECS components related to identification
#ifndef IDENTIFICATION_H
#define IDENTIFICATION_H

// Dungeon Divers (avoid name collisions)
namespace DD
{
	struct Player {};
	struct Bullet {};
	struct CountBullet{};
	struct Enemy {};
	struct Lives {};
	struct Treasure {};
	struct Heart {};
	struct ControllerID
	{
		unsigned index = 0;
	};

	struct Name
	{
		std::string name = "";
	};
};

#endif