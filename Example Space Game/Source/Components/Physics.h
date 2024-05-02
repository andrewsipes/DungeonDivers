// define all ECS components related to movement & collision
#ifndef PHYSICS_H
#define PHYSICS_H

// Dungeon Divers (avoid name collisions)
namespace DD 
{
	// ECS component types should be *strongly* typed for proper queries
	// typedef is tempting but it does not help templates/functions resolve type
	struct Position { GW::MATH2D::GVECTOR2F value; };
	struct Velocity { GW::MATH2D::GVECTOR2F value; };
	struct Orientation { GW::MATH2D::GMATRIX2F value; };
	struct Acceleration { GW::MATH2D::GVECTOR2F value; };

	struct World { GW::MATH::GMATRIXF value; };
	struct LastWorld { GW::MATH::GMATRIXF value; };
	struct OGPos { GW::MATH::GMATRIXF value; };
	struct BulletVel { GW::MATH::GVECTORF value; };
	struct EnemyVel { GW::MATH::GVECTORF value; };
	struct IFrame { float value; };
	struct MoveCooldown { float value; };
	struct State { GW::MATH::GVECTORF value; };

	// Individual TAGs
	struct Collidable { GW::MATH::GOBBF obb; };

	// ECS Relationship tags
	struct CollidedWith {};
};

#endif