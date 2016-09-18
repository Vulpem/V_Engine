#ifndef __MODULE_GUI__
#define __MODULE_GUI__

#include "Module.h"
#include "Globals.h"

union SDL_Event;

class Panel;
class PanelConsole;
class PanelGOTree;
class PanelProperties;
class PanelConfiguration;
class PanelAbout;
class PanelResources;
class PanelQuickBar;
class GameObject;

class ModuleGUI : public Module
{
public:
	
	ModuleGUI(Application* app, bool start_enabled = true);
	~ModuleGUI();

	bool Init();

	bool Start();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);

	bool CleanUp();

	void HandleInput(SDL_Event* event);

	bool testWindowOpen = true;

	bool capture_mouse = false;
	bool capture_keyboard = false;
	bool in_modal = false;
	bool draw_menu = true;

	char* tmpInput;
};

#endif