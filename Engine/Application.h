#ifndef __APP__
#define __APP__

#include "Globals.h"
#include "Timer.h"
#include "C_String.h"

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
class ModuleScene;
class ModuleFileSystem;
class ModuleImporter;

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
	ModuleImporter* importer;
	ModuleGoManager* GO;
	ModulePhysics3D* physics;

	ModuleScene* scene;

	float ms_frame[EDITOR_FRAME_SAMPLES];
	float framerate[EDITOR_FRAME_SAMPLES];
	int maxFPS = 0;

private:

	Timer	ms_timer;
	float	dt;
	std::vector<Module*> list_modules;
	int frameCount = 0;
	Timer	FPS_Timer;
	float FrameTime = -1.0f;
	int previous_maxFPS = maxFPS;
	bool gameRunning = false;

	C_String title;
	C_String organisation;

public:

	Application();
	~Application();

	Timer totalTimer;
	bool Init();
	update_status Update();
	bool CleanUp();

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