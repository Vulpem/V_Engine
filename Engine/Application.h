#ifndef __APP__
#define __APP__

#include "Globals.h"
#include "Timer.h"
#include "PerfTimer.h"

#include <vector>
#include <list>

class Module;
class ModuleWindow;
class ModuleInput;
class ModuleAudio;
class ModuleRenderer3D;
class ModuleCamera3D;
class ModuleEditor;
class ModuleGoManager;
class ModulePhysics3D;
class ModuleFileSystem;
class ModuleImporter;
class ModuleResourceManager;
class TimerManager;

struct viewPort;

#include "MathGeoLib\include\MathGeoLibFwd.h"

class Application
{
public:
	ModuleWindow* window;
	ModuleInput* input;
	ModuleAudio* audio;
	ModuleFileSystem* fs;

	ModuleRenderer3D* renderer3D;
	ModuleCamera3D* camera;

	ModuleEditor* Editor;
	ModuleResourceManager* resources;
	ModuleImporter* importer;
	ModuleGoManager* GO;
	ModulePhysics3D* physics;

	float ms_frame[EDITOR_FRAME_SAMPLES];
	float framerate[EDITOR_FRAME_SAMPLES];
	int maxFPS = 0;

	TimerManager* timers;

private:

	PerfTimer	ms_timer;
	std::vector<Module*> list_modules;
	int frameCount = 0;
	Timer	FPS_Timer;
	float FrameTime = -1.0f;
	int previous_maxFPS = maxFPS;
	bool gameRunning = false;

	std::string title;
	std::string organisation;

public:

	Application();
	~Application();

	Timer totalTimer;
	bool Init();
	update_status Update();
	bool CleanUp();

	void Render(const viewPort& port);

	bool OpenBrowser(const char* link);
	void Log(char* str);

	const char* GetOrganization();
	const char* GetTitle();

	void OnScreenResize(int width, int heigth);
private:

	void AddModule(Module* mod);
	void PrepareUpdate();
	void FinishUpdate();
};

extern Application* App;

#endif