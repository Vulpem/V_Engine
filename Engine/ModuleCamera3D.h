#ifndef __CAMERA_3D__
#define __CAMERA_3D__

#include "Module.h"
#include "Globals.h"
#include "glmath.h"
#include "Camera.h"


class ModuleCamera3D : public Module
{
public:
	ModuleCamera3D(Application* app, bool start_enabled = true);
	~ModuleCamera3D();

	bool Start();
	update_status Update(float dt);
	bool CleanUp();

	void Look(const vec3 &Position, const vec3 &Reference, bool RotateAroundReference = false);
	void LookAt(const vec3 &Spot);
	void Move(const vec3 &Movement);
	void SetPos(const vec3 &Pos);
	float* GetViewMatrix();

	void SetActiveCamera(Camera* activeCamera);
	void SetActiveCamera(GameObject* activeCamera);
	void SetCameraToDefault();
	Camera* GetActiveCamera();

	void UpdateView();

private:

	void CalculateViewMatrix();

public:
	
	float distanceToRef = 20.f;
	vec3 X, Y, Z, Position, Reference;
	float camSpeed = 0.4f;
	float camSprintMultiplier = 2.0f;

private:
	GameObject* defaultCameraGO = nullptr;
	Camera* defaultCamera = nullptr;
	Camera* activeCamera = nullptr;

	mat4x4 ViewMatrix, ViewMatrixInverse;
};

#endif