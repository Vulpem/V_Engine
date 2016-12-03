#ifndef __MODULE_RESOURCE_MANAGER__
#define __MODULE_RESOURCE_MANAGER__

#include "Module.h"
#include "Globals.h"

#include "Component.h"

#include <map>

class Resource;

struct MetaInf
{
	std::string name;
	uint64_t uid;
	Component::Type type;
};

struct R_Folder
{
	R_Folder(const char* name, R_Folder* parent);
	R_Folder(const char* name, const char* path);

	std::string name;
	std::string path;

	std::vector<std::string> subFoldersPath;
	std::vector<std::string> files;
};


class ModuleResourceManager : public Module
{
public:
	
	ModuleResourceManager(Application* app, bool start_enabled = true);
	~ModuleResourceManager();

	bool Start();
	update_status PreUpdate();
	update_status PostUpdate();
	bool CleanUp();

private:
	std::map<uint64_t, Resource*> resources;
	std::map<Component::Type, std::map<std::string, uint64_t>> uidLib;

	std::vector<uint64_t> toDelete;

	std::map<std::string, std::vector<MetaInf>> metaData;

	//R_Folder resBaseFolder;

	Resource* LoadNewResource(std::string fileName);

public:
	void ReimportAll();
	void ClearLibrary();

private:
	//void RefreshFolder(const char* path);

	//Warning, this folder will be incomplete. It will have no parent or path
	R_Folder ReadFolder(const char* path);

	//void CreateFolderMeta(R_Folder& folder);
	//R_Folder ReadFolderMeta(const char* path);
public:
	Resource* Peek(uint64_t uid) const;

	Resource* LinkResource(uint64_t uid) ;
	uint64_t LinkResource(std::string fileName, Component::Type type);

	void UnlinkResource(Resource* res);
	void UnlinkResource(uint64_t uid);
	void UnlinkResource(std::string fileName, Component::Type type);

	void DeleteNow();

	const std::vector<Resource*> ReadLoadedResources() const;
};

#endif