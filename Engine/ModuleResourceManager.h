#ifndef __MODULE_RESOURCE_MANAGER__
#define __MODULE_RESOURCE_MANAGER__

#include "Module.h"
#include "Globals.h"
#include "AllComponents.h"
#include <map>



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
	Resource* LinkResource(uint64_t uid);
	Resource* LinkResource(std::string fileName, Component::Type type);

	void UnlinkResource(Resource* res);
	void UnlinkResource(uint64_t uid);
	void UnlinkResource(std::string fileName, Component::Type type);

	void DeleteNow();

	const std::map<uint64_t, Resource*>& ReadLoadedResources() const;
};

#endif