#pragma once

#include "p2List.h"
#include "Globals.h"
#include "Timer.h"
#include "Module.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleAudio.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModulePhysics3D.h"
#include "ModuleUI.h"
#include "ModuleTests.h"
#include "ModuleScene.h"
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

private:

	Timer	ms_timer;
	float	dt;
	p2List<Module*> list_modules;

public:

	Application();
	~Application();

	Timer totalTimer;
	bool Init();
	update_status Update();
	bool CleanUp();

	bool OpenBrowser(const char* link);

private:

	void AddModule(Module* mod);
	void PrepareUpdate();
	void FinishUpdate();
};