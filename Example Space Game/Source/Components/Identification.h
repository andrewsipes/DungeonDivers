// define all ECS components related to identification
#ifndef IDENTIFICATION_H
#define IDENTIFICATION_H

// Dungeon Divers (avoid name collisions)
namespace DD
{
	struct Player {};
	struct Bullet {};
	struct EnemyBullet {};
	struct CountBullet {};
	struct EnemyCountBullet {};
	struct Enemy {};
	struct MushEnemy {};
	struct SpikeEnemy {};
	struct BeholdEnemy {};
	struct AmDead { float value; };
	struct Lives {};
	struct Treasure {};
	struct Heart {};
	struct Destruct {};
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