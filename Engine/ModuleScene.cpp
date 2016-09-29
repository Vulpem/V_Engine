#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "Primitive.h"
#include "PhysBody3D.h"
#include "ModuleCamera3D.h"

#include "ModuleImportGeometry.h"


ModuleScene::ModuleScene(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

ModuleScene::~ModuleScene()
{}

// Load assets
bool ModuleScene::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	App->camera->LookAt(vec3(0, 7, 0));

	mesh* tmp = App->importGeometry->LoadFBX("FBX/Mecha.fbx");
	tmp->SetPos(20, -5, 10);

	reset = false;
	return ret;
}

// Load assets
bool ModuleScene::CleanUp()
{
	
	return true;
}

// Update
update_status ModuleScene::Update(float dt)
{
	P_Plane p(0, 0, 0, 1);
	p.axis = true;
	p.Render();

	return UPDATE_CONTINUE;
}

update_status ModuleScene::PostUpdate(float dt)
{
	if (reset)
	{
		ResetScene();
		reset = false;
	}

	return UPDATE_CONTINUE;
}

void ModuleScene::OnCollision(PhysBody3D* body1, PhysBody3D* body2)
{

}


void ModuleScene::ResetScene()
{
	App->camera->Disable();
	this->Disable();
	this->Enable();
	App->camera->Enable();
}
