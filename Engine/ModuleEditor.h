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
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);

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
	void Editor();
	void Console();
	void Outliner();
	void AttributeWindow();
	void SwitchViewPorts();
	void ViewPortUI(const viewPort& port);

	void SelectByViewPort();

public:
	bool multipleViews = true;

	bool IsOpenTestWindow = false;

	bool showPlane = false;
	bool showAxis = true;
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
	uint previewViewPort = 0;

	ImGuiTextBuffer buffer;
	bool scrollToBottom;

	char toImport[256];
	char testConsoleInput[256];
	C_String importResult;
	math::float3 changeGeometryPos;

	GameObject* selectedGameObject = nullptr;
	float selectedPos[3] = { 0,0,0 };
	float selectedScale[3] = { 1,1,1 };
	float selectedEuler[3] = { 0,0,0 };
};

#endif