#ifndef __CAMERA_3D__
#define __CAMERA_3D__

#include "Module.h"
#include "Globals.h"
#include "Camera.h"


class ModuleCamera3D : public Module
{
public:
	ModuleCamera3D(Application* app, bool start_enabled = true);
	~ModuleCamera3D();

	bool Start();
	update_status Update();
	bool CleanUp();

	void LookAt(const float3 &Spot);
	void Move(const float3 &Movement);
	void SetPos(const float3 &Pos);

	Camera* GetDefaultCam();
	Camera* GetTopCam();
	Camera* GetRightCam();
	Camera* GetFrontCam();

	void SetCameraToDefault(Camera* toSet);
	void SetCameraToTop(Camera* toSet);
	void SetCameraToRight(Camera* toSet);
	void SetCameraToFront(Camera* toSet);
	void SetCameraToCamera(GameObject* setTo, Camera* toSet);

	Camera* GetMovingCamera();
	void SetMovingCamera(Camera* cam = nullptr);
private:
	void MoveWithKeys();

public:
	float camSpeed = 30.0f;
	float camSprintMultiplier = 3.0f;

private:
	Camera* movingCamera = nullptr;
	GameObject* defaultCameraGO = nullptr;
	Camera* defaultCamera = nullptr;

	GameObject* topView = nullptr;
	GameObject* rightView = nullptr;
	GameObject* frontView = nullptr;
public:
};

#endif