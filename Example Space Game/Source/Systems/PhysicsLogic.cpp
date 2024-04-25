#include "PhysicsLogic.h"
#include "../Components/Physics.h"
#include "../Components/Identification.h"

bool ESG::PhysicsLogic::Init(std::shared_ptr<flecs::world> _game, std::weak_ptr<const GameConfig> _gameConfig)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;

	// **** MOVEMENT ****
	// update velocity by acceleration
	game->system<Velocity, const Acceleration>("Acceleration System")
		.each([](flecs::entity e, Velocity& v, const Acceleration& a)
			{
				GW::MATH2D::GVECTOR2F accel;
				GW::MATH2D::GVector2D::Scale2F(a.value, e.delta_time(), accel);
				GW::MATH2D::GVector2D::Add2F(accel, v.value, v.value);
			});

	// update position by velocity
	game->system<Position, const Velocity>("Translation System")
		.each([](flecs::entity e, Position& p, const Velocity& v)
			{
				GW::MATH2D::GVECTOR2F speed;
				GW::MATH2D::GVector2D::Scale2F(v.value, e.delta_time(), speed);

				// adding is simple but doesn't account for orientation
				GW::MATH2D::GVector2D::Add2F(speed, p.value, p.value);
			});

	// **** CLEANUP ****
	// clean up any objects that end up offscreen
	game->system<const Position>("Cleanup System")
		.each([](flecs::entity e, const Position& p)
			{
				if (p.value.x > 1.5f || p.value.x < -1.5f || p.value.y > 1.5f || p.value.y < -1.5f)
				{
					e.destruct();
				}
			});

	// **** COLLISIONS ****
	// due to wanting to loop through all collidables at once, we do this in two steps:
	// 1. A System will gather all collidables into a shared std::vector
	// 2. A second system will run after, testing/resolving all collidables against each other
	queryCache = game->query<Collidable, World>();

	// only happens once per frame at the very start of the frame
	struct CollisionSystem {}; // local definition so we control iteration count (singular)
	game->entity("Detect-Collisions").add<CollisionSystem>();

	game->system<CollisionSystem>()
		.each([this](CollisionSystem& s)
			{
				// This the base shape all objects use & draw, this might normally be a component collider.(ex:sphere/box)
				/*constexpr GW::MATH2D::GVECTOR2F poly[polysize] =
				{
					{ -0.5f, -0.5f }, { 0, 0.5f }, { 0.5f, -0.5f }, { 0, -0.25f }
				};
				GW::MATH::GVECTORF boundary[8]; <<---- equivalent to the above*/

				//constexpr GW::MATH::GVECTORF square[boxSize] =
				//{
				//	{ -1.0f, -1.0f, -1.0f }, // Bottom-back-left corner
				//	{ 1.0f, -1.0f, -1.0f },  // Bottom-back-right corner
				//	{ 1.0f, 1.0f, -1.0f },   // Top-back-right corner
				//	{ -1.0f, 1.0f, -1.0f }   // Top-back-left corner
				//};

				std::cout << queryCache.count() << std::endl;

				// collect any and all collidable objects
				queryCache.each([&](flecs::entity e, Collidable& c, World& w)
					{
						// create a 3x3 matrix for transformation
						/*GW::MATH2D::GMATRIX3F matrix =
						{
							o.value.row1.x, o.value.row1.y, 0,
							o.value.row2.x, o.value.row2.y, 0,
							p.value.x, p.value.y, 1
						};*/

						/*GW::MATH::GMATRIXF matrix =
						{
							w.value.row1.x, w.value.row1.y, 0,
							w.value.row2.x, w.value.row2.y, 0,
							w.value.row4.x, w.value.row4.y, 1
						};*/

						GW::MATH::GMATRIXF matrix = w.value;

						SHAPE box; // compute buffer for this objects polygon

						// This is critical, if you want to store an entity handle it must be mutable
						box.owner = e; // allows later changes
						box.obby = c.obb;
						box.obby.center = w.value.row4;
						
						/*for (int i = 0; i < 8; ++i)
						{
							GW::MATH2D::GVECTOR3F v = { poly[i].x, poly[i].y, 1 };
							GW::MATH2D::GMatrix2D::MatrixXVector3F(matrix, v, v);
							polygon.poly[i].x = v.x;
							polygon.poly[i].y = v.y;
						}*/

						/*for (int i = 0; i < boxSize; ++i)
						{
							GW::MATH::GVECTORF v = { square[i].x, square[i].y, 1 };
							std::cout << "Original Point " << i << ": (" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;

							GW::MATH::GMatrix::VectorXMatrixF(matrix, v, v);
							std::cout << "Transformed Point " << i << ": (" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;

							box.boundingBox[i].x = v.x;
							box.boundingBox[i].y = v.y;
						}*/

						/*for (int i = 0; i < 8; i++)
						{
							GW::MATH::GVECTORF v = { box.obby.center.x, box.obby.extent.y, box.obby.rotation.z };
							GW::MATH::GMatrix::VectorXMatrixF(matrix, v, v);
							box.obby.center.x = v.x;
							box.obby.extent.y = v.y;
							box.obby.rotation.z = v.z;
						}*/

						//attempt 2
						/*for (int i = 0; i < 8; i++)
						{
							GW::MATH::GVECTORF v = { w.value.row1.x, w.value.row2.y, 1 };
							GW::MATH::GMatrix::VectorXMatrixF(matrix, v, v);
							box.obby.center.x = v.x;
							box.obby.extent.y = v.y;
						}*/

						//attempt 3
						/*for (int i = 0; i < 8; i++)
						{
							// Define a vertex vector
							GW::MATH::GVECTORF vertex;

							// Assign the coordinates of the vertex from the boundary array
							vertex.x = Model::boundary[i].x;
							vertex.y = Model::boundary[i].y;
							vertex.z = Model::boundary[i].z;

							// Apply the transformation matrix to the vertex
							GW::MATH::GMatrix::VectorXMatrixF(matrix, vertex, vertex);

							// Update the transformed vertex coordinates in the bounding box
							if (i < 4) 
							{
								box.obby.extent.x = std::fmax(box.obby.extent.x, std::abs(vertex.x - box.obby.center.x));
								box.obby.extent.y = std::fmax(box.obby.extent.y, std::abs(vertex.y - box.obby.center.y));
								box.obby.extent.z = std::fmax(box.obby.extent.z, std::abs(vertex.z - box.obby.center.z));
							}
							else 
							{
								box.obby.extent.x = std::fmax(box.obby.extent.x, std::abs(vertex.x - box.obby.center.x));
								box.obby.extent.y = std::fmax(box.obby.extent.y, std::abs(vertex.y - box.obby.center.y));
								box.obby.extent.z = std::fmax(box.obby.extent.z, std::abs(vertex.z - box.obby.center.z));
							}
						}*/

						// add to vector
						testCache.push_back(box);
					});

				// loop through the testCahe resolving all collisions
				for (int i = 0; i < testCache.size(); ++i)
				{
					// the inner loop starts at the entity after you so you don't double check collisions
					for (int j = i + 1; j < testCache.size(); ++j)
					{
						// test the two world space polygons for collision
						// possibly make this cheaper by leaving one of them local and using an inverse matrix
						GW::MATH::GCollision::GCollisionCheck results;
						GW::MATH::GCollision::TestOBBToOBBF(testCache[i].obby, testCache[j].obby, results);

						if (results == GW::MATH::GCollision::GCollisionCheck::COLLISION)
						{
							// Create an ECS relationship between the colliders
							// Each system can decide how to respond to this info independently
							testCache[j].owner.add<CollidedWith>(testCache[i].owner);
							testCache[i].owner.add<CollidedWith>(testCache[j].owner);

							std::cout << "Collision Detected between:  1: " << testCache[i].owner.get<Name>()->name << "    2. " << testCache[j].owner.get<Name>()->name << std::endl;
						}
					}
				}
				// wipe the test cache for the next frame (keeps capacity intact)
				testCache.clear();
			});

	return true;
}

bool ESG::PhysicsLogic::Activate(bool runSystem)
{
	if (runSystem)
	{
		game->entity("Acceleration System").enable();
		game->entity("Translation System").enable();
		game->entity("Cleanup System").enable();
	}
	else
	{
		game->entity("Acceleration System").disable();
		game->entity("Translation System").disable();
		game->entity("Cleanup System").disable();
	}
	return true;
}

bool ESG::PhysicsLogic::Shutdown()
{
	queryCache.destruct(); // fixes crash on shutdown
	game->entity("Acceleration System").destruct();
	game->entity("Translation System").destruct();
	game->entity("Cleanup System").destruct();
	return true;
}