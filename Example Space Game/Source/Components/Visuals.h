// define all ECS components related to drawing
#ifndef VISUALS_H
#define VISUALS_H

// Dungeon Divers (avoid name collisions)
namespace DD
{
	struct Color { GW::MATH2D::GVECTOR3F value; };

	struct Material {
		Color diffuse = { 1, 1, 1 };
	};
};

#endif