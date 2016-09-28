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

void ModuleImportGeometry::LoadFBX(char* path)
{
	const aiScene* scene = aiImportFile(path, aiProcessPreset_TargetRealtime_Fast);
	if (scene != nullptr && scene->HasMeshes())
	{
		for (uint n = 0; n < scene->mNumMeshes; n++)
		{
			aiMesh* impMesh = scene->mMeshes[n];

			mesh toPush;

			aiVector3D* currentVertex = impMesh->mVertices;

			toPush.num_vertices = impMesh->mNumVertices;
			toPush.vertices = new float(toPush.num_vertices * 3);
			for (int v = 0; v < toPush.num_vertices; n += 3)
			{
				toPush.vertices[v] = currentVertex->x;
				toPush.vertices[v + 1] = currentVertex->y;
				toPush.vertices[v + 2] = currentVertex->z;
				currentVertex++;
			}

			toPush.num_vertices = impMesh->mNumFaces * 3;
			toPush.indices = new uint(toPush.num_indices);
			for (int v = 0; v < toPush.num_vertices; n += 3)
			{
				toPush.vertices[v] = currentVertex->x;
				toPush.vertices[v + 1] = currentVertex->y;
				toPush.vertices[v + 2] = currentVertex->z;
				currentVertex++;
			}

			impMesh->mFaces->mNumIndices

			meshes.push_back(toPush);
		}
		aiReleaseImport(scene);
	}
	else
	{
		LOG("Error loading scene %s", path);
	}
}