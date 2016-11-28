#ifndef __MODULE_RESOURCE_MANAGER__
#define __MODULE_RESOURCE_MANAGER__

#include "Module.h"
#include "Globals.h"
#include "AllComponents.h"
#include <map>
#include <queue>

class Resource
{
public:
	Resource() { uid = GenerateUUID(); }
	Resource(uint64_t UID) { this->uid = UID; }

	//Number of references to this resource. Won't unload while it's over 0
	uint nReferences = 0;

	//Unique ID for this resource
	uint64_t uid = 0;

	//Filename this resource extraced the data from
	std::string file;
};

class R_mesh : public Resource
{
public:
	R_mesh():Resource() {}
	R_mesh(uint64_t UID) : Resource(UID) { aabb.SetNegativeInfinity(); }

	~R_mesh();

	static Component::Type GetType() { return Component::Type::C_mesh; }

	float3* vertices = nullptr;
	uint id_vertices = 0;
	uint num_vertices = 0;

	uint* indices = nullptr;
	uint id_indices = 0;
	uint num_indices = 0;

	float3* normals = nullptr;
	uint id_normals = 0;
	uint num_normals;

	uint id_textureCoords = 0;
	uint num_textureCoords = 0;

	int defaultMaterialIndex = -1;

	AABB aabb;
};


class ModuleResourceManager : public Module
{
public:
	
	ModuleResourceManager(Application* app, bool start_enabled = true);
	~ModuleResourceManager();

	bool Init();
	update_status PreUpdate();
	update_status PostUpdate();
	bool CleanUp();

private:
	std::map<uint64_t, Resource*> resources;
	std::map<std::string, uint64_t> uidLib;

	std::queue<uint64_t> toDelete;

	Resource* LoadNewResource(std::string fileName);

public:
	void UnlinkResource(Resource* res);
	void UnlinkResource(uint64_t uid);
	void UnlinkResource(std::string fileName);

	void DeleteNow();

	Resource* LinkResource(uint64_t uid)
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

	Resource* LinkResource(std::string fileName)
	{
		Resource* ret = nullptr;
		std::map<std::string, uint64_t>::iterator it = uidLib.find(fileName);
		if (it != uidLib.end())
		{
			ret = GetResource(it->second);
		}

		if (ret == nullptr)
		{

			Resource* ret = LoadNewResource(fileName);
			if (ret != nullptr)
			{
				resources.insert(std::pair<uint64_t, Resource*>(ret->uid, res));
				uidLib.insert(std::pair<std::string, uint64_t>(ret->file, ret->uid));
				ret->nReferences++;
			}
		}

		return ret;
	}

};

#endif