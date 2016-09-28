#ifndef __MODULE_GUI__
#define __MODULE_GUI__

#include "Module.h"
#include "Globals.h"



class ModuleImportGeometry : public Module
{
public:
	
	ModuleImportGeometry(Application* app, bool start_enabled = true);
	~ModuleImportGeometry();

	bool Init();

	bool Start();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);

	bool CleanUp();
};

#endif