#ifndef __MODULE_UI__
#define __MODULE_UI__

#include "Module.h"
#include "Globals.h"

#include "Math.h"
#include "ViewPort.h"

#include "ImGui\imgui.h"

class GameObject;

class ModuleEditor : public Module
{
public:
	
	ModuleEditor(Application* app, bool start_enabled = true);
	~ModuleEditor();

	bool Init();

	bool Start();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();

	bool CleanUp();

	void Render(const viewPort& port);

	void OnScreenResize(int width, int heigth);

	void HandleInput(SDL_Event* event);

	void ModuleEditor::Log(const char* input);
	void ClearConsole();
private:
	void SceneTreeGameObject(GameObject* node);
	void SelectGameObject(GameObject* node);

	update_status MenuBar();
	void PlayButtons();
	void Editor();
	void Console();
	void Outliner();
	void AttributeWindow();
	void SwitchViewPorts();
	void ViewPortUI(const viewPort& port);
	bool SaveLoadPopups();

	void SelectByViewPort();

public:
	bool multipleViews = false;

	bool IsOpenTestWindow = false;

	bool showPlane = true;
	bool renderNormals = false;

	//TMP
	LineSegment selectRay;
	float3 out_normal;
	float3 out_pos;

private:
	int screenW = 0;
	int screenH = 0;

	float2 viewPortMin;
	float2 viewPortMax;
	uint singleViewPort = 0;
	uint multipleViewPorts[4] = { 0,0,0,0 };

	ImGuiTextBuffer buffer;
	bool scrollToBottom;

	char toImport[256];
	char testConsoleInput[256];
	std::string importResult;
	math::float3 changeGeometryPos;

	GameObject* selectedGameObject = nullptr;
	float selectedPos[3] = { 0,0,0 };
	float selectedScale[3] = { 1,1,1 };
	float selectedEuler[3] = { 0,0,0 };

	bool wantNew = false;
	bool wantToSave = false;
	bool wantToLoad = false;
	bool clearAfterSave = false;
	char sceneName[256];
};

#endif