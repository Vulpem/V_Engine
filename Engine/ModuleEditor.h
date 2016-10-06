#ifndef __MODULE_UI__
#define __MODULE_UI__

#include "Module.h"
#include "Globals.h"

#include "Math.h"

#include "ImGui\imgui.h"

class GameObject;

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

class ModuleEditor : public Module
{
public:
	
	ModuleEditor(Application* app, bool start_enabled = true);
	~ModuleEditor();

	bool Init();

	bool Start();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);

	bool CleanUp();

	void HandleInput(SDL_Event* event);

	void ModuleEditor::Log(const char* input);
	void ClearConsole();
private:
	void SceneTreeGameObject(GameObject* node);
	void SelectGameObject(GameObject* node);

public:

	bool capture_mouse = false;
	bool capture_keyboard = false;

	bool IsOpenConsole = true;
	bool IsOpenEditor = true;
	bool IsOpenTestWindow = false;
	bool IsOpenOutliner = true;
	bool IsOpenAttributes = true;

	bool showPlane = true;
	bool showAxis = true;
	bool renderNormals = false;

	char* testConsoleInput;

	float camRef[3] = { 0,0,0 };

private:
	ImGuiTextBuffer buffer;
	bool scrollToBottom;

	std::vector<GameObject*> geometries;
	char toImport[256];
	C_String importResult;
	math::float3 changeGeometryPos;

	GameObject* selectedGameObject = nullptr;
	float selectedPos[3] = { 0,0,0 };
	float selectedScale[3] = { 1,1,1 };
	float selectedEuler[3] = { 0,0,0 };
};

#endif