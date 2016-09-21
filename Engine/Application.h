#pragma once

#include "p2List.h"
#include "Globals.h"
#include "Timer.h"

class Module;
class ModuleWindow;
class ModuleInput;
class ModuleAudio;
class ModuleRenderer3D;
class ModuleCamera3D;
class ModuleUI;
class ModuleTests;
class ModulePhysics3D;
class ModuleScene;

#include "MathGeoLib\include\MathGeoLibFwd.h"

class Application
{
public:
	ModuleWindow* window;
	ModuleInput* input;
	ModuleAudio* audio;

	ModuleRenderer3D* renderer3D;
	ModuleCamera3D* camera;

	ModuleUI* UI;
	ModuleTests* test;
	ModulePhysics3D* physics;

	ModuleScene* scene;

	float ms_frame[EDITOR_FRAME_SAMPLES];
	float framerate[EDITOR_FRAME_SAMPLES];
	int maxFPS = 0;

private:

	Timer	ms_timer;
	float	dt;
	p2List<Module*> list_modules;
	int frameCount = 0;
	Timer	FPS_Timer;
	float FrameTime = -1.0f;
	int previous_maxFPS = maxFPS;

public:

	Application();
	~Application();

	Timer totalTimer;
	bool Init();
	update_status Update();
	bool CleanUp();

	bool OpenBrowser(const char* link);
	void Log(char* str);

private:

	void AddModule(Module* mod);
	void PrepareUpdate();
	void FinishUpdate();
};

//extern Application* App;