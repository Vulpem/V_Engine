#include "Globals.h"
#include "Application.h"
#include "PhysBody3D.h"
#include "Primitive.h"
#include "ModuleCamera3D.h"
#include "ModuleInput.h"
#include "ModuleGOmanager.h"

#include "Transform.h"

ModuleCamera3D::ModuleCamera3D(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name.create("ModuleCamera3D");
}

ModuleCamera3D::~ModuleCamera3D()
{}

// -----------------------------------------------------------------
bool ModuleCamera3D::Start()
{
	LOG("Setting up the camera");
	bool ret = true;
	defaultCameraGO = App->GO->CreateCamera("DefaultEditorCamera");
	defaultCamera = defaultCameraGO->GetComponent<Camera>().front();
	defaultCameraGO->HideFromOutliner();

	topView = App->GO->CreateCamera("TopView");
	topView->GetTransform()->SetLocalPos(0, 1000, 0);
	topView->GetTransform()->SetLocalRot(90, 0, 0);
	topView->GetTransform()->allowRotation = false;
	topView->GetComponent<Camera>().front()->SetFarPlane(2000);
	topView->GetComponent<Camera>().front()->SwitchViewType();
	topView->HideFromOutliner();

	frontView = App->GO->CreateCamera("FrontView");
	frontView->GetTransform()->SetLocalPos(0, 0, -1000);
	frontView->GetTransform()->SetLocalRot(0, 0, 0);
	frontView->GetTransform()->allowRotation = false;
	frontView->GetComponent<Camera>().front()->SetFarPlane(2000);
	frontView->GetComponent<Camera>().front()->SwitchViewType();
	frontView->HideFromOutliner();

	rightView = App->GO->CreateCamera("RightView");
	rightView->GetTransform()->SetLocalPos(-1000, 0, 0);
	rightView->GetTransform()->SetLocalRot(0, 90, 0);
	rightView->GetTransform()->allowRotation = false;
	rightView->GetComponent<Camera>().front()->SetFarPlane(2000);
	rightView->GetComponent<Camera>().front()->SwitchViewType();
	rightView->HideFromOutliner();

	return ret;
}

// -----------------------------------------------------------------
bool ModuleCamera3D::CleanUp()
{
	LOG("Cleaning camera");

	return true;
}

// -----------------------------------------------------------------
update_status ModuleCamera3D::Update(float dt)
{

	MoveWithKeys();
	// Mouse motion ----------------
	if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT)
	{
		int dx = -App->input->GetMouseXMotion();
		int dy = -App->input->GetMouseYMotion();
		if (dx != 0 || dy != 0)
		{
			float Sensitivity = 0.05f;

			Transform* cam = GetMovingCamera()->object->GetTransform();
			
			float3 toLook = cam->GetGlobalPos();
			toLook += cam->Forward() * 10;

			toLook += dy * Sensitivity * cam->Up();

			toLook += dx * Sensitivity * cam->Left();

			LookAt(toLook);
		}
	}

	return UPDATE_CONTINUE;
}


// -----------------------------------------------------------------
void ModuleCamera3D::LookAt( const float3 &Spot)
{
	GetMovingCamera()->object->GetTransform()->LookAt(Spot);
}


// -----------------------------------------------------------------
void ModuleCamera3D::Move(const float3 &Movement)
{
	GetMovingCamera()->object->GetTransform()->SetGlobalPos(GetMovingCamera()->object->GetTransform()->GetGlobalPos() + Movement);
}

void ModuleCamera3D::SetPos(const float3 &Pos)
{
	GetMovingCamera()->object->GetTransform()->SetGlobalPos(Pos);
}

// -----------------------------------------------------------------

Camera * ModuleCamera3D::GetTopCam()
{
	return topView->GetComponent<Camera>().front();
}

Camera * ModuleCamera3D::GetRightCam()
{
	return rightView->GetComponent<Camera>().front();
}

Camera * ModuleCamera3D::GetFrontCam()
{
	return frontView->GetComponent<Camera>().front();
}

void ModuleCamera3D::SetActiveCamera(Camera * activeCamera)
{
	this->activeCamera = activeCamera;
	if (this->activeCamera)
	{
		this->activeCamera->frustumChanged = true;
	}
	else
	{
		defaultCamera->frustumChanged = true;
	}
}

void ModuleCamera3D::SetActiveCamera(GameObject * activeCamera)
{
	std::vector<Camera*> cam = activeCamera->GetComponent<Camera>();
	if (cam.empty() == false)
	{
		SetActiveCamera(cam.front());
	}
}

void ModuleCamera3D::SetCameraToDefault()
{
	SetActiveCamera((Camera*)nullptr);
}

void ModuleCamera3D::SetCameraToTop()
{
	SetCameraToDefault();
	SetCameraToCamera(topView);
}

void ModuleCamera3D::SetCameraToRight()
{
	SetCameraToDefault();
	SetCameraToCamera(rightView);
}

void ModuleCamera3D::SetCameraToFront()
{
	SetCameraToDefault();
	SetCameraToCamera(frontView);
}

void ModuleCamera3D::SetCameraToCamera(GameObject * setTo)
{
	if (setTo->GetComponent<Camera>().empty() == false && setTo->GetTransform() != nullptr)
	{
		GetActiveCamera()->object->GetTransform()->SetLocalPos(setTo->GetTransform()->GetLocalPos());
		GetActiveCamera()->object->GetTransform()->SetLocalRot(setTo->GetTransform()->GetLocalRot());
		if (GetActiveCamera()->GetFrustum()->type != setTo->GetComponent<Camera>().front()->GetFrustum()->type)
		{
			GetActiveCamera()->SwitchViewType();
		}
	}
}


Camera * ModuleCamera3D::GetMovingCamera()
{
	Camera* cam = movingCamera;
	if (cam == nullptr)
	{
		cam = GetActiveCamera();
	}
	return cam;
}

void ModuleCamera3D::SetMovingCamera(Camera * cam)
{
	movingCamera = cam;
}

Camera * ModuleCamera3D::GetActiveCamera()
{
	if (activeCamera != nullptr)
	{
		return activeCamera;
	}
	return defaultCamera;
}

float3 ModuleCamera3D::GetCamPos()
{
	return GetActiveCamera()->object->GetTransform()->GetGlobalPos();
}

void ModuleCamera3D::MoveWithKeys()
{
	Camera* cam = GetMovingCamera();

	float speed = camSpeed;
	float3 lastCamPos = cam->object->GetTransform()->GetGlobalPos();
	float3 camPos = lastCamPos;
	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
	{
		speed *= camSprintMultiplier;
	}

	int mouseWheel = App->input->GetMouseZ();
	if (mouseWheel != 0)
	{
		if (cam->GetFrustum()->type == FrustumType::PerspectiveFrustum)
		{
			lastCamPos += cam->object->GetTransform()->Forward() * speed * mouseWheel * 10;
		}
		else
		{
			cam->SetHorizontalFOV(cam->GetFrustum()->horizontalFov - speed * mouseWheel * 10);
		}
	}

	//Forward Backward
	//In Ortographic mode, moving the camera forward or backward is meaningless. Instead we'll change the FOV to change the zoom lvl
	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
	{
		if (cam->GetFrustum()->type == FrustumType::PerspectiveFrustum)
		{
			lastCamPos += cam->object->GetTransform()->Forward() * speed;
		}
		else
		{
			cam->SetHorizontalFOV(cam->GetFrustum()->horizontalFov - speed);
		}
	}
	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
	{
		if (cam->GetFrustum()->type == FrustumType::PerspectiveFrustum)
		{
			lastCamPos += cam->object->GetTransform()->Backward() * speed;
		}
		else
		{
			cam->SetHorizontalFOV(cam->GetFrustum()->horizontalFov + speed);
		}
	}

	//Right Left
	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		lastCamPos += cam->object->GetTransform()->Left() * speed;
	}
	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{
		lastCamPos += cam->object->GetTransform()->Right() * speed;
	}

	//Up Down
	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
	{
		lastCamPos += cam->object->GetTransform()->Down() * speed;
	}
	if (App->input->GetKey(SDL_SCANCODE_E) == KEY_REPEAT)
	{
		lastCamPos += cam->object->GetTransform()->Up() * speed;
	}

	if (lastCamPos.x != camPos.x || lastCamPos.y != camPos.y || lastCamPos.z != camPos.z)
	{
		cam->object->GetTransform()->SetGlobalPos(lastCamPos);
	}
}
