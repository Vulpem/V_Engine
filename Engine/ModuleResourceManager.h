#ifndef __MODULE_RESOURCE_MANAGER__
#define __MODULE_RESOURCE_MANAGER__

#include "Module.h"
#include "Globals.h"
#include "AllComponents.h"
#include <map>

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

	virtual Component::Type GetType() = 0;

	template <typename T>
	const T* Read() { return (T*)this; }
};

class R_Material : public Resource
{
public:
	R_Material() :Resource() { }
	R_Material(uint64_t UID) : Resource(UID) {}

	~R_Material();

	Component::Type GetType() { return Component::Type::C_material; }

	std::vector<uint> textures;
	std::vector<std::string> texturePaths;

	float color[4] = { 1.0f,1.0f,1.0f,1.0f };
};

class R_mesh : public Resource
{
public:
	R_mesh() :Resource() { aabb.SetNegativeInfinity(); }
	R_mesh(uint64_t UID) : Resource(UID) { aabb.SetNegativeInfinity(); }

	~R_mesh();

	Component::Type GetType() { return Component::Type::C_mesh; }

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
	std::map<std::pair<Component::Type, std::string>, uint64_t> uidLib;

	std::vector<uint64_t> toDelete;

	Resource* LoadNewResource(std::string fileName);

public:
	void UnlinkResource(Resource* res);
	void UnlinkResource(uint64_t uid);
	void UnlinkResource(std::string fileName, Component::Type type);

	void DeleteNow();

	const std::map<uint64_t, Resource*>& ReadLoadedResources() const;

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

	Resource* LinkResource(std::string fileName, Component::Type type)
	{
		Resource* ret = nullptr;
		std::map<std::pair<Component::Type, std::string>, uint64_t>::iterator it = uidLib.find(std::pair<Component::Type, std::string>(type, fileName));
		if (it != uidLib.end())
		{
			ret = LinkResource(it->second);
		}

		if (ret == nullptr)
		{
			ret = LoadNewResource(fileName);
			if (ret != nullptr)
			{
				resources.insert(std::pair<uint64_t, Resource*>(ret->uid, ret));
				std::pair<Component::Type, std::string> tmp(ret->GetType(), ret->file);
				uidLib.insert(std::pair<std::pair<Component::Type, std::string>, uint64_t>(tmp, ret->uid));
				ret->nReferences++;
			}
		}
		return ret;
	}

};

#endif