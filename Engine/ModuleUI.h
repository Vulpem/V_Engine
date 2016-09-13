#ifndef __MODULE_GUI__
#define __MODULE_GUI__

#include "Module.h"
#include "Globals.h"

#include "ModuleRenderer3D.h"

class ModuleGUI : public Module
{
public:
	
	ModuleGUI(Application* app, bool start_enabled = true);
	~ModuleGUI();

	bool Init();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);

	bool CleanUp();

	bool testWindowOpen = true;
};

#endif