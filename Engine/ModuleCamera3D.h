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

	void LookAt(const float3 &Spot);
	void Move(const float3 &Movement);
	void SetPos(const float3 &Pos);
	float* GetViewMatrix();
	float* GetProjectionMatrix();

	void SetActiveCamera(Camera* activeCamera);
	void SetActiveCamera(GameObject* activeCamera);
	void SetCameraToDefault();
	void AddCamCulling(Camera* toAdd);
	void RemoveCamCulling(Camera* toRemove);
	void ClearCamCulling();

	Camera* GetActiveCamera();
	float3 GetCamPos();
private:
	void MoveWithKeys();

public:
	float camSpeed = 0.4f;
	float camSprintMultiplier = 2.0f;

private:
	GameObject* defaultCameraGO = nullptr;
	Camera* defaultCamera = nullptr;
	Camera* activeCamera = nullptr;

	GameObject* topView = nullptr;
	GameObject* rightView = nullptr;
	GameObject* frontView = nullptr;
public:
	std::vector<Camera*> cullingCameras;
};

#endif