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
	struct BoundingBox { GW::MATH2D::GRECTANGLE2D rectCollider; };

	// Individual TAGs
	struct Collidable {}; 

	// Component for representing a bounding box
	//struct BoundingBox 
	//{
	//	// Define the bounding box type
	//	GW::MATH2D::GRECTANGLE2D rectangleCollider;

	//	// Collision check object
	//	//GW::MATH2D::GCollision2D collisionCheck;
	//};
	
	// ECS Relationship tags
	struct CollidedWith {};
};

#endif