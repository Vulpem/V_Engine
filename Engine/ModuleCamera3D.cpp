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
	// Mouse motion ----------------
	bool updatePos = false;

#pragma region cameraMovementKeys
	float speed = camSpeed;
	float3 lastCamPos = GetActiveCamera()->object->GetTransform()->GetGlobalPos();
	float3 camPos = lastCamPos;
	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
	{
		speed *= camSprintMultiplier;
	}
	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
	{
		lastCamPos += GetActiveCamera()->object->GetTransform()->GetGlobalTransform().Transposed().WorldZ() * speed;
	}
	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
	{
		lastCamPos -= GetActiveCamera()->object->GetTransform()->GetGlobalTransform().Transposed().WorldZ() * speed;
	}
	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		lastCamPos += GetActiveCamera()->object->GetTransform()->GetGlobalTransform().Transposed().WorldX() * speed;
	}
	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{
		lastCamPos -= GetActiveCamera()->object->GetTransform()->GetGlobalTransform().Transposed().WorldX() * speed;
	}
	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
	{
		lastCamPos -= GetActiveCamera()->object->GetTransform()->GetGlobalTransform().Transposed().WorldY() * speed;
	}
	if (App->input->GetKey(SDL_SCANCODE_E) == KEY_REPEAT)
	{
		lastCamPos += GetActiveCamera()->object->GetTransform()->GetGlobalTransform().Transposed().WorldY() * speed;
	}
	if (lastCamPos.x != camPos.x || lastCamPos.y != camPos.y || lastCamPos.z != camPos.z)
	{
		GetActiveCamera()->object->GetTransform()->SetGlobalPos(lastCamPos);
	}
#pragma endregion
	if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT || updatePos)
	{
		int dx = -App->input->GetMouseXMotion();
		int dy = -App->input->GetMouseYMotion();

		float Sensitivity = 0.15f;		
	}

	return UPDATE_CONTINUE;
}


// -----------------------------------------------------------------
void ModuleCamera3D::LookAt( const float3 &Spot)
{
	float4x4 tmp;
	Transform* activeTrans = GetActiveCamera()->object->GetTransform();
	if (GetActiveCamera()->object->parent && GetActiveCamera()->object->parent->HasComponent(Component::Type::C_transform))
	{
		 tmp = float4x4::LookAt(activeTrans->GetGlobalPos(), Spot, activeTrans->GetGlobalTransform().WorldZ(), activeTrans->GetGlobalTransform().WorldY(), float3(0,1,0));
	}
	else
	{
		tmp = float4x4::LookAt(activeTrans->GetGlobalPos(), Spot, activeTrans->GetGlobalTransform().WorldY(), float3(0,1,0));
	}	
	Quat quat(tmp);
	activeTrans->SetLocalRot(quat.x, quat.y, quat.z, quat.z);
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
	float4x4 ret(GetActiveCamera()->GetFrustum()->ViewMatrix());
	return ret.Transposed().ptr();
}

void ModuleCamera3D::SetActiveCamera(Camera * activeCamera)
{
	this->activeCamera = activeCamera;
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
