#include "Globals.h"
#include "Application.h"

#include "ModuleResourceManager.h"

#include "ModuleImporter.h"
#include "R_Resource.h"


ModuleResourceManager::ModuleResourceManager(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name = "ModuleResourceManager";
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
	std::map<uint64_t, Resource*>::iterator it = resources.begin();
	for (; it != resources.end(); it++)
	{
		RELEASE(it->second);
	}

	resources.clear();
	uidLib.clear();

	return true;
}

Resource * ModuleResourceManager::LoadNewResource(std::string fileName)
{
	std::string format(".");
	format += App->importer->FileFormat(fileName.data());
	if (format == MESH_FORMAT)
	{
		return (Resource*)App->importer->LoadMesh(fileName.data());
	}	
	if (format == MATERIAL_FORMAT)
	{
		return (Resource*)App->importer->LoadMaterial(fileName.data());
	}
	if (format == TEXTURE_FORMAT)
	{
		return (Resource*)App->importer->LoadTexture(fileName.data());
	}
	return nullptr;
}

Resource * ModuleResourceManager::LinkResource(uint64_t uid)
{
	Resource* ret = nullptr;
	std::map<uint64_t, Resource*>::iterator it = resources.find(uid);
	if (it != resources.end())
	{
		ret = it->second;
		ret->nReferences++;
	}
	return ret;
}

Resource * ModuleResourceManager::LinkResource(std::string fileName, Component::Type type)
{
	Resource* ret = nullptr;
	std::map<Component::Type, std::map<std::string, uint64_t>>::iterator tmpMap = uidLib.find(type);
	if (tmpMap == uidLib.end())
	{
		uidLib.insert(std::pair<Component::Type, std::map<std::string, uint64_t>>(type, std::map<std::string, uint64_t>()));
		tmpMap = uidLib.find(type);
	}
	std::map<std::string, uint64_t> ::iterator it = tmpMap->second.find(fileName);

	if (it != tmpMap->second.end())
	{
		ret = LinkResource(it->second);
	}

	if (ret == nullptr)
	{
		ret = LoadNewResource(fileName);
		if (ret != nullptr)
		{
			resources.insert(std::pair<uint64_t, Resource*>(ret->uid, ret));
			tmpMap->second.insert(std::pair<std::string, uint64_t>(ret->file, ret->uid));
			ret->nReferences++;
		}
	}
	return ret;
}

void ModuleResourceManager::UnlinkResource(Resource * res)
{
	UnlinkResource(res->uid);
}

void ModuleResourceManager::UnlinkResource(uint64_t uid)
{
	std::map<uint64_t, Resource*>::iterator it = resources.find(uid);
	if (it != resources.end())
	{
		it->second->nReferences--;
		if (it->second->nReferences <= 0)
		{
			toDelete.push_back(it->first);
		}
	}
}

void ModuleResourceManager::UnlinkResource(std::string fileName, Component::Type type)
{
	std::map<Component::Type, std::map<std::string, uint64_t>>::iterator tmpMap = uidLib.find(type);
	if (tmpMap != uidLib.end())
	{
		std::map<std::string, uint64_t>::iterator it = tmpMap->second.find(fileName);
		if (it != tmpMap->second.end())
		{
			UnlinkResource(it->second);
		}
	}
}

void ModuleResourceManager::DeleteNow()
{
	if (toDelete.empty() == false)
	{
		std::vector<uint64_t> tmp = toDelete;
		toDelete.clear();
		while (tmp.empty() == false)
		{
			uint64_t uid = tmp.back();

			//Erasing the resource itself
			std::map<uint64_t, Resource*>::iterator it = resources.find(uid);
			if (it != resources.end())
			{
				RELEASE(it->second);
				resources.erase(it);
			}

			//TODO
			//When metadata is created, we won't erase from here anymore

			/*bool found = false;
			//Erasing its reference in the uid Lib
			std::map<Component::Type, std::map<std::string, uint64_t>>::iterator tmpMap = uidLib.begin();
			for (; tmpMap != uidLib.end() && found == false; tmpMap++)
			{
				std::map<std::string, uint64_t>::iterator it = tmpMap->second.begin();
				for (;it != tmpMap->second.end() && found == false; it++)
				{
					if (it->second == uid)
					{
						tmpMap->second.erase(it);
						found = true;
					}
				}
			}*/
			tmp.pop_back();
		}
	}
}

const std::vector<Resource*> ModuleResourceManager::ReadLoadedResources() const
{
	std::vector<Resource*> ret;

	std::map<Component::Type, std::map<std::string, uint64_t>>::const_iterator tmpMap = uidLib.cbegin();
	for (; tmpMap != uidLib.end(); tmpMap++)
	{
		std::map<std::string, uint64_t>::const_iterator it = tmpMap->second.cbegin();
		for (; it != tmpMap->second.end(); it++)
		{
			std::map<uint64_t, Resource*>::const_iterator res = resources.find(it->second);
			if (res != resources.cend())
			{
				ret.push_back(res->second);
			}
		}
	}

	return ret;
}
