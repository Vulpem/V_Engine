#include "Globals.h"
#include "Application.h"

#include "ModuleImportGeometry.h"


#include "Assimp\include\cimport.h"
#include "Assimp\include\scene.h"
#include "Assimp\include\postprocess.h"
#include "Assimp\include\cfileio.h"

#pragma comment(lib, "Assimp/libx86/assimp.lib")


ModuleImportGeometry::ModuleImportGeometry(Application* app, bool start_enabled) : Module(app, start_enabled)
{

}

// Destructor
ModuleImportGeometry::~ModuleImportGeometry()
{

}

// Called before render is available
bool ModuleImportGeometry::Init()
{
	bool ret = true;

	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);
		
	return ret;
}

bool ModuleImportGeometry::Start()
{
	
	return true;
}

// Called every draw update
update_status ModuleImportGeometry::PreUpdate(float dt)
{
	update_status ret = UPDATE_CONTINUE;

	return ret;
}

update_status ModuleImportGeometry::Update(float dt)
{
	
	return UPDATE_CONTINUE;
}

update_status ModuleImportGeometry::PostUpdate(float dt)
{
	
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleImportGeometry::CleanUp()
{
	aiDetachAllLogStreams();

	return true;
}