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

	defaultCameraGO = App->GO->CreateCamera();
	defaultCamera = *defaultCameraGO->GetComponent<Camera>().begin();
	defaultCameraGO->SetName("Default editor camera");

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
			float Sensitivity = 0.15f;

			Transform* activeCam = GetActiveCamera()->object->GetTransform();

			
			float3 toLook = activeCam->GetGlobalPos();
			toLook += activeCam->GetGlobalTransform().WorldZ() * 10;

			toLook.y += dy * Sensitivity;

			LookAt(toLook);
		}
	}

	return UPDATE_CONTINUE;
}


// -----------------------------------------------------------------
void ModuleCamera3D::LookAt( const float3 &Spot)
{
	GetActiveCamera()->object->GetTransform()->LookAt(Spot);
}


// -----------------------------------------------------------------
void ModuleCamera3D::Move(const float3 &Movement)
{
	GetActiveCamera()->object->GetTransform()->SetGlobalPos(GetActiveCamera()->object->GetTransform()->GetGlobalPos() + Movement);
}

void ModuleCamera3D::SetPos(const float3 &Pos)
{
	GetActiveCamera()->object->GetTransform()->SetGlobalPos(Pos);
}

// -----------------------------------------------------------------
float* ModuleCamera3D::GetViewMatrix()
{
	return GetActiveCamera()->GetViewMatrix().Transposed().ptr();
}

float* ModuleCamera3D::GetProjectionMatrix()
{
	return GetActiveCamera()->GetProjectionMatrix().ptr();
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
		SetActiveCamera(*cam.begin());
	}
}

void ModuleCamera3D::SetCameraToDefault()
{
	SetActiveCamera((Camera*)nullptr);
}

void ModuleCamera3D::AddCamCulling(Camera * toAdd)
{
	cullingCameras.push_back(toAdd);
}

void ModuleCamera3D::RemoveCamCulling(Camera * toRemove)
{
	if (cullingCameras.empty() == false)
	{
		std::vector<Camera*>::iterator it = cullingCameras.begin();
		while (it != cullingCameras.end())
		{
			if ((*it) == toRemove)
			{
				cullingCameras.erase(it);
				return;
			}
			it++;
		}
	}
}

void ModuleCamera3D::ClearCamCulling()
{
	cullingCameras.clear();
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
	float speed = camSpeed;
	float3 lastCamPos = GetActiveCamera()->object->GetTransform()->GetGlobalPos();
	float3 camPos = lastCamPos;
	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
	{
		speed *= camSprintMultiplier;
	}
	Camera* cam = GetActiveCamera();

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
