#include "Globals.h"
#include "Application.h"

#include "ModuleResourceManager.h"

#include "ModuleImporter.h"

#include "OpenGL.h"




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

update_status ModuleResourceManager::PostUpdate()
{
	DeleteNow();
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleResourceManager::CleanUp()
{
	return true;
}

Resource * ModuleResourceManager::LoadNewResource(std::string fileName)
{
	return App->importer->LoadMesh(fileName.data());
	return nullptr;
}

void ModuleResourceManager::UnlinkResource(Resource * res)
{
	UnlinkResource(res->uid);
}

void ModuleResourceManager::UnlinkResource(uint64_t uid)
{
	std::map<uint64_t, Resource*>::iterator it = resources.find(uid);
	it->second->nReferences--;
	if (it->second->nReferences <= 0)
	{
		toDelete.push(it->first);
	}
}

void ModuleResourceManager::UnlinkResource(std::string fileName)
{
	std::map<std::string, uint64_t>::iterator it = uidLib.find(fileName);
	if (it != uidLib.end())
	{
		UnlinkResource(it->second);
	}
}

void ModuleResourceManager::DeleteNow()
{
	while (toDelete.empty() == false)
	{
		uint64_t uid = toDelete.front();

		for (std::map<std::string, uint64_t>::iterator it = uidLib.begin(); it != uidLib.end(); it++)
		{
			if (it->second == uid)
			{
				uidLib.erase(it);
				break;
			}
		}
		
		std::map<uint64_t, Resource*>::iterator it = resources.find(uid);
		if (it != resources.end())
		{
			RELEASE(it->second);
			resources.erase(it);
		}
	}
}

R_mesh::~R_mesh()
{
	if (id_indices != 0)
	{
		glDeleteBuffers(1, &id_indices);
	}
	if (id_normals != 0)
	{
		glDeleteBuffers(1, &id_normals);
	}
	if (id_textureCoords != 0)
	{
		glDeleteBuffers(1, &id_textureCoords);
	}
	if (id_vertices != 0)
	{
		glDeleteBuffers(1, &id_vertices);
	}

	RELEASE_ARRAY(vertices);
	RELEASE_ARRAY(indices);
	RELEASE_ARRAY(normals);
}
