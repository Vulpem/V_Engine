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

	CalculateViewMatrix();

	X = vec3(1.0f, 0.0f, 0.0f);
	Y = vec3(0.0f, 1.0f, 0.0f);
	Z = vec3(0.0f, 0.0f, 1.0f);

	Position = vec3(0.0f, 20.0f, -10.0f);
	Reference = vec3(0.0f, 0.0f, 0.0f);
}

ModuleCamera3D::~ModuleCamera3D()
{}

// -----------------------------------------------------------------
bool ModuleCamera3D::Start()
{
	LOG("Setting up the camera");
	bool ret = true;
	X = vec3(1.0f, 0.0f, 0.0f);
	Y = vec3(0.0f, 1.0f, 0.0f);
	Z = vec3(0.0f, 0.0f, 1.0f);

	Position = vec3(0.0f, 20.0f, -10.0f);
	Reference = vec3(0.0f, 0.0f, 0.0f);

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
	if (App->input->GetMouseButton(4) == KEY_DOWN)
	{
		distanceToRef++;
		updatePos = true;
	}
	else if(App->input->GetMouseButton(5) == KEY_DOWN)
	{
		distanceToRef++;
		updatePos = true;
	}

#pragma region cameraMovementKeys
	float speed = camSpeed;
	float3 lastCamPos = defaultCameraGO->GetTransform()->GetGlobalPos();
	float3 camPos = lastCamPos;
	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
	{
		speed *= camSprintMultiplier;
	}
	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
	{
		lastCamPos -= defaultCameraGO->GetTransform()->GetGlobalTransform().Transposed().WorldZ() * speed;
	}
	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
	{
		lastCamPos += defaultCameraGO->GetTransform()->GetGlobalTransform().Transposed().WorldZ() * speed;
	}
	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		lastCamPos -= defaultCameraGO->GetTransform()->GetGlobalTransform().Transposed().WorldX() * speed;
	}
	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{
		lastCamPos += defaultCameraGO->GetTransform()->GetGlobalTransform().Transposed().WorldX() * speed;
	}
	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
	{
		lastCamPos -= defaultCameraGO->GetTransform()->GetGlobalTransform().Transposed().WorldY() * speed;
	}
	if (App->input->GetKey(SDL_SCANCODE_E) == KEY_REPEAT)
	{
		lastCamPos += defaultCameraGO->GetTransform()->GetGlobalTransform().Transposed().WorldY() * speed;
	}
	if (lastCamPos.x != camPos.x || lastCamPos.y != camPos.y || lastCamPos.z != camPos.z)
	{
		defaultCameraGO->GetTransform()->SetGlobalPos(lastCamPos);
	}
#pragma endregion
	if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT || updatePos)
	{
		int dx = -App->input->GetMouseXMotion();
		int dy = -App->input->GetMouseYMotion();

		float Sensitivity = 0.15f;		
	}
	

	// Recalculate matrix -------------
	CalculateViewMatrix();

	return UPDATE_CONTINUE;
}

// -----------------------------------------------------------------
void ModuleCamera3D::Look(const vec3 &Position, const vec3 &Reference, bool RotateAroundReference)
{
	this->Position = Position;
	this->Reference = Reference;

	Z = normalize(Position - Reference);
	X = normalize(cross(vec3(0.0f, 1.0f, 0.0f), Z));
	Y = cross(Z, X);

	if (!RotateAroundReference)
	{
		this->Reference = this->Position;
		this->Position += Z * 0.05f;
	}

	CalculateViewMatrix();
}

// -----------------------------------------------------------------
void ModuleCamera3D::LookAt( const vec3 &Spot)
{
	Reference = Spot;

	Z = normalize(Position - Reference);
	X = normalize(cross(vec3(0.0f, 1.0f, 0.0f), Z));
	Y = cross(Z, X);

	Position = Reference + Z * distanceToRef;

	CalculateViewMatrix();
}


// -----------------------------------------------------------------
void ModuleCamera3D::Move(const vec3 &Movement)
{
	Position += Movement;
	Reference += Movement;

	CalculateViewMatrix();
}

void ModuleCamera3D::SetPos(const vec3 &Pos)
{
	Position = Pos;
	LookAt(Reference);
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

Camera * ModuleCamera3D::GetActiveCamera()
{
	if (activeCamera != nullptr)
	{
		return activeCamera;
	}
	return defaultCamera;
}

// -----------------------------------------------------------------
void ModuleCamera3D::CalculateViewMatrix()
{
	ViewMatrix = mat4x4(X.x, Y.x, Z.x, 0.0f, X.y, Y.y, Z.y, 0.0f, X.z, Y.z, Z.z, 0.0f, -dot(X, Position), -dot(Y, Position), -dot(Z, Position), 1.0f);
	ViewMatrixInverse = inverse(ViewMatrix);
}

void ModuleCamera3D::UpdateView()
{
	Position = Reference + Z * distanceToRef;

	CalculateViewMatrix();
}