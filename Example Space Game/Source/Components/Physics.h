// define all ECS components related to movement & collision
#ifndef PHYSICS_H
#define PHYSICS_H

// example space game (avoid name collisions)
namespace ESG 
{
	// ECS component types should be *strongly* typed for proper queries
	// typedef is tempting but it does not help templates/functions resolve type
	struct Position { GW::MATH2D::GVECTOR2F value; };
	struct Velocity { GW::MATH2D::GVECTOR2F value; };
	struct Orientation { GW::MATH2D::GMATRIX2F value; };
	struct Acceleration { GW::MATH2D::GVECTOR2F value; };

	struct World { GW::MATH::GMATRIXF value; };
	struct BulletVel { GW::MATH::GVECTORF value; };
	//struct BoundingBox { GW::MATH::GOBBF obb; };

	struct Wall {};
	struct Floor {};
	struct Stationary {};

	// Individual TAGs
	struct Collidable { GW::MATH::GOBBF obb; };
	//struct Collidable {};

	// ECS Relationship tags
	struct CollidedWith {};
};

#endif