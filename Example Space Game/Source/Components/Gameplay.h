// define all ECS components related to gameplay
#ifndef GAMEPLAY_H
#define GAMEPLAY_H

// Dungeon Divers (avoid name collisions)
namespace DD
{
	struct Damage { int value; };
	struct Health { int value; };
	struct Firerate { float value; };
	struct Cooldown { float value; };
	
	// gameplay tags (states)
	struct Firing {};
	struct Charging {};

	// powerups
	struct ChargedShot { int max_destroy; };
};

#endif