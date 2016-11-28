#ifndef __MODULE_RESOURCE_MANAGER__
#define __MODULE_RESOURCE_MANAGER__

#include "Module.h"
#include "Globals.h"


class ModuleResourceManager : public Module
{
public:
	
	ModuleResourceManager(Application* app, bool start_enabled = true);
	~ModuleResourceManager();

	bool Init();
	update_status PreUpdate();
	bool CleanUp();
};

#endif