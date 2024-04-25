// define all ECS components related to identification
#ifndef IDENTIFICATION_H
#define IDENTIFICATION_H

// example space game (avoid name collisions)
namespace ESG
{
	struct Player {};
	struct Bullet {};
	struct CountBullet{};
	struct Enemy {};
	struct Lives {};
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