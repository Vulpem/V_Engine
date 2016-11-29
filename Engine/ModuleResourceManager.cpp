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
	std::string format(".");
	format += App->importer->FileFormat(fileName.data());
	if (format == MESH_FORMAT)
	{
		return App->importer->LoadMesh(fileName.data());
	}	
	if (format == MATERIAL_FORMAT)
	{
		return App->importer->LoadMaterial(fileName.data());
	}
	if (format == TEXTURE_FORMAT)
	{
		return App->importer->LoadTexture(fileName.data());
	}
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
		toDelete.push_back(it->first);
	}
}

void ModuleResourceManager::UnlinkResource(std::string fileName, Component::Type type)
{
	std::map<std::pair<Component::Type, std::string>, uint64_t>::iterator it = uidLib.find(std::pair<Component::Type, std::string>(type, fileName));
	if (it != uidLib.end())
	{
		UnlinkResource(it->second);
	}
}

void ModuleResourceManager::DeleteNow()
{
	while (toDelete.empty() == false)
	{
		uint64_t uid = toDelete.back();

		std::map<uint64_t, Resource*>::iterator it = resources.find(uid);
		if (it != resources.end())
		{
			RELEASE(it->second);
			resources.erase(it);
		}

		for (std::map<std::pair<Component::Type, std::string>, uint64_t>::iterator it = uidLib.begin(); it != uidLib.end(); it++)
		{
			if (it->second == uid)
			{
				uidLib.erase(it);
				break;
			}
		}
		
		toDelete.pop_back();
	}
}

const std::map<uint64_t, Resource*>& ModuleResourceManager::ReadLoadedResources() const
{
	return resources;
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

R_Material::~R_Material()
{
	if (textures.empty() == false)
	{
		for (std::vector<R_Texture*>::iterator it = textures.begin(); it != textures.end(); it++)
		{
			App->resources->UnlinkResource(*it);
		}
	}
}

R_Texture::~R_Texture()
{
	glDeleteBuffers(1, &bufferID);
}
