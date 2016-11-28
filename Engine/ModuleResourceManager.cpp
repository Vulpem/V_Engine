#include "Globals.h"
#include "Application.h"

#include "ModuleResourceManager.h"

#include "ModuleImporter.h"




ModuleResourceManager::ModuleResourceManager(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name.create("ModuleResourceManager");
}

// Destructor
ModuleResourceManager::~ModuleResourceManager()
{
}

// Called before render is available
bool ModuleResourceManager::Init()
{
	return true;
}

// Called every draw update
update_status ModuleResourceManager::PreUpdate()
{
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleResourceManager::CleanUp()
{
	return true;
}