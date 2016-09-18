#ifndef __MODULE_GUI__
#define __MODULE_GUI__

#include "Module.h"
#include "Globals.h"
#include "Math.h"

class ModuleTests : public Module
{
public:
	
	ModuleTests(Application* app, bool start_enabled = true);
	~ModuleTests();

	bool Init();

	bool Start();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);

	bool CleanUp();
};

#endif