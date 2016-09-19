#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "Primitive.h"
#include "PhysBody3D.h"


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

	App->camera->SetPos(vec3(10.0f, 10.0f, 10.0f));
	
	App->camera->LookAt(vec3(0, 0, 0));
	loadNext = false;
	ended = false;
	loadNext = false;
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
	if (loadNext)
	{
		//LoadNextScene();
		loadNext = false;
	}

	return UPDATE_CONTINUE;
}

void ModuleScene::OnCollision(PhysBody3D* body1, PhysBody3D* body2)
{

}

void ModuleScene::LoadPositions()
{}

void ModuleScene::ResetScene()
{
	App->camera->Disable();
	this->Disable();
	this->Enable();
	App->camera->Enable();
}
