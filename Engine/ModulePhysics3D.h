#ifndef __MODULE_PHYSICS_3D__
#define __MODULE_PHYSICS_3D__

#include "Module.h"
#include "Globals.h"
#include "Primitive.h"

#include "Math.h"

#include "Bullet\include\btBulletDynamicsCommon.h"

// Recommended scale is 1.0f == 1 meter, no less than 0.2 objects
#define GRAVITY btVector3(0.0f, -10.0f, 0.0f) 

class DebugDrawer;
struct PhysBody3D;

class ModulePhysics3D : public Module
{
public:
	ModulePhysics3D(Application* app, bool start_enabled = true);
	~ModulePhysics3D();

	bool Init();
	bool Start();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	PhysBody3D* AddBody(const P_Sphere& sphere, float mass = 1.0f);
	PhysBody3D* AddBody(const P_Cube& cube, float mass = 1.0f);
	PhysBody3D* AddBody(const P_Cylinder& cylinder, float mass = 1.0f);


	void AddConstraintP2P(PhysBody3D& bodyA, PhysBody3D& bodyB, const math::float3& anchorA, const math::float3& anchorB);
	void AddConstraintHinge(PhysBody3D& bodyA, PhysBody3D& bodyB, const math::float3& anchorA, const math::float3& anchorB, const math::float3& axisS, const math::float3& axisB, bool disable_collision = false);
	void DeleteBody(PhysBody3D* body);

private:

	bool debug = false;

	btDefaultCollisionConfiguration*	collision_conf;
	btCollisionDispatcher*				dispatcher;
	btBroadphaseInterface*				broad_phase;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld*			world;
	DebugDrawer*						debug_draw;

	std::list<btCollisionShape*> shapes;
	std::list<PhysBody3D*> bodies;
	std::list<btDefaultMotionState*> motions;
	std::list<btTypedConstraint*> constraints;

	btRigidBody* ground = nullptr;
};

class DebugDrawer : public btIDebugDraw
{
public:
	DebugDrawer() : line(0,0,0)
	{}

	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
	void reportErrorWarning(const char* warningString);
	void draw3dText(const btVector3& location, const char* textString);
	void setDebugMode(int debugMode);
	int	 getDebugMode() const;

	DebugDrawModes mode;
	P_Line line;
	Primitive point;
};

#endif