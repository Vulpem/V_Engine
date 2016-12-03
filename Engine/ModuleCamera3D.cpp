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
	moduleName = "ModuleCamera3D";
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
update_status ModuleCamera3D::Update()
{

	MoveWithKeys();
	// Mouse motion ----------------
	if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT)
	{
		int dx = -App->input->GetMouseXMotion();
		int dy = -App->input->GetMouseYMotion();
		if (dx != 0 || dy != 0)
		{
			float Sensitivity = 0.04;
			Transform* cam = GetMovingCamera()->object->GetTransform();
			
			float3 toLook = cam->GetGlobalPos();
			toLook += cam->Forward() * 10;

			toLook += (float)dy * Sensitivity *cam->Up();

			toLook += (float)dx * Sensitivity * cam->Left();

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

Camera * ModuleCamera3D::GetDefaultCam()
{
	return defaultCamera;
}

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

void ModuleCamera3D::SetCameraToDefault(Camera* toSet)
{
	SetCameraToCamera(defaultCameraGO, toSet);
}

void ModuleCamera3D::SetCameraToTop(Camera* toSet)
{
	SetCameraToCamera(topView, toSet);
}

void ModuleCamera3D::SetCameraToRight(Camera* toSet)
{
	SetCameraToCamera(rightView, toSet);
}

void ModuleCamera3D::SetCameraToFront(Camera* toSet)
{
	SetCameraToCamera(frontView, toSet);
}

void ModuleCamera3D::SetCameraToCamera(GameObject * setTo, Camera* toSet)
{
	if (setTo->HasComponent(Component::Type::C_camera) && setTo->GetTransform() != nullptr)
	{
		toSet->object->GetTransform()->SetLocalPos(setTo->GetTransform()->GetLocalPos());
		toSet->object->GetTransform()->SetLocalRot(setTo->GetTransform()->GetLocalRot());
		if (toSet->GetFrustum()->type != setTo->GetComponent<Camera>().front()->GetFrustum()->type)
		{
			toSet->SwitchViewType();
		}
	}
}


Camera * ModuleCamera3D::GetMovingCamera()
{
	Camera* cam = movingCamera;
	if (cam == nullptr)
	{
		cam = GetDefaultCam();
	}
	return cam;
}

void ModuleCamera3D::SetMovingCamera(Camera * cam)
{
	movingCamera = cam;
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
			lastCamPos += cam->object->GetTransform()->Forward() * speed * mouseWheel * 10 * Time.dt;
		}
		else
		{
			cam->SetHorizontalFOV(cam->GetFrustum()->horizontalFov - speed * mouseWheel * 10 * Time.dt);
		}
	}

	//Forward Backward
	//In Ortographic mode, moving the camera forward or backward is meaningless. Instead we'll change the FOV to change the zoom lvl
	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
	{
		if (cam->GetFrustum()->type == FrustumType::PerspectiveFrustum)
		{
			lastCamPos += cam->object->GetTransform()->Forward() * speed* Time.dt;
		}
		else
		{
			cam->SetHorizontalFOV(cam->GetFrustum()->horizontalFov - speed * Time.dt);
		}
	}
	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
	{
		if (cam->GetFrustum()->type == FrustumType::PerspectiveFrustum)
		{
			lastCamPos += cam->object->GetTransform()->Backward() * speed* Time.dt;
		}
		else
		{
			cam->SetHorizontalFOV(cam->GetFrustum()->horizontalFov + speed* Time.dt);
		}
	}

	//Right Left
	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		lastCamPos += cam->object->GetTransform()->Left() * speed* Time.dt;
	}
	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{
		lastCamPos += cam->object->GetTransform()->Right() * speed* Time.dt;
	}

	//Up Down
	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
	{
		lastCamPos += cam->object->GetTransform()->Down() * speed* Time.dt;
	}
	if (App->input->GetKey(SDL_SCANCODE_E) == KEY_REPEAT)
	{
		lastCamPos += cam->object->GetTransform()->Up() * speed* Time.dt;
	}

	if (lastCamPos.x != camPos.x || lastCamPos.y != camPos.y || lastCamPos.z != camPos.z)
	{
		cam->object->GetTransform()->SetGlobalPos(lastCamPos);
	}
}
