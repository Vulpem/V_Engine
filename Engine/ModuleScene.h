#pragma once
#include "Module.h"
#include "Globals.h"
#include "Primitive.h"
#include "PhysBody3D.h"


struct PhysBody3D;
struct PhysMotor3D;

class ModuleScene : public Module
{
public:
	ModuleScene(Application* app, bool start_enabled = true);
	~ModuleScene();

	bool Start();
	update_status Update(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();

	void OnCollision(PhysBody3D* body1, PhysBody3D* body2);

	void ResetScene();

public:
	bool reset = false;
};