#ifndef __MODULE_UI__
#define __MODULE_UI__

#include "Module.h"
#include "Globals.h"

#include "ImGui\imgui.h"

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

class ModuleUI : public Module
{
public:
	
	ModuleUI(Application* app, bool start_enabled = true);
	~ModuleUI();

	bool Init();

	bool Start();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);

	bool CleanUp();

	void HandleInput(SDL_Event* event);

	void ModuleUI::Log(const char* input);
	void ClearConsole();



	

	bool capture_mouse = false;
	bool capture_keyboard = false;
	bool in_modal = false;
	bool draw_menu = true;

	bool IsOpenConsole = true;
	bool IsOpenEditor = true;
	bool IsOpenTestWindow = true;

	char* tmpInput;

private:
	ImGuiTextBuffer buffer;
	bool scrollToBottom;
};

#endif