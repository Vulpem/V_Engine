#include "Application.h"

#include <time.h>


#include "Module.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleAudio.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModulePhysics3D.h"
#include "ModuleEditor.h"
#include "ModuleGoManager.h"
#include "ModuleFileSystem.h"
#include "ModuleImporter.h"
#include "ModuleResourceManager.h"

#include "Timers.h"

Application::Application()
{
	gameRunning = false;

	title = TITLE;
	organisation = ORGANISATION;

	window = new ModuleWindow(this);
	input = new ModuleInput(this);
	audio = new ModuleAudio(this, true);
	fs = new ModuleFileSystem(this);

	renderer3D = new ModuleRenderer3D(this);
	camera = new ModuleCamera3D(this);
	physics = new ModulePhysics3D(this);
#if USE_EDITOR
		Editor = new ModuleEditor(this);
#endif
	resources = new ModuleResourceManager(this);
	GO = new ModuleGoManager(this);
	importer = new ModuleImporter(this);

	timers = new TimerManager();

	// The order of calls is very important!
	// Modules will Init() Start() and Update in this order
	// They will CleanUp() in reverse order

	// Main Modules
	AddModule(window);
	AddModule(input);
	AddModule(camera);
#if USE_EDITOR
		AddModule(Editor);
#endif
	AddModule(fs);
	AddModule(audio);
	AddModule(physics);
	AddModule(importer);
	AddModule(resources);
	AddModule(GO);

	// Renderer last!
	AddModule(renderer3D);

	for (int n = 0; n < EDITOR_FRAME_SAMPLES; n++)
	{
		ms_frame[n] = 0;
		framerate[n] = 0;
	}
	FrameTime = -1.0f;

	srand(time(NULL));
}

Application::~Application()
{
	std::vector<Module*>::reverse_iterator item = list_modules.rbegin();

	while(item != list_modules.rend())
	{
		delete *item;
		item++;
	}
}

bool Application::Init()
{
	bool ret = true;
	totalTimer.Start();
	// Call Init() in all modules
	std::vector<Module*>::iterator item = list_modules.begin();

	while(item != list_modules.end() && ret == true)
	{
		ret = (*item)->Init();
		item++;
	}

	// After all Init calls we call Start() in all modules
	LOG("Application Start --------------");
	item = list_modules.begin();

	//Variable used to determine if LOG's can be shown on console
	gameRunning = true;

	while(item != list_modules.end() && ret == true)
	{
		if ((*item)->IsEnabled())
		{
			ret = (*item)->Start();
		}
		item++;
	}
	maxFPS = 0;

	ms_timer.Start();
	FPS_Timer.Start();

	//TMP
	TIMER_CREATE("__Timer");
	TIMER_CREATE_PERF("__PerfTimer");
	TIMER_CREATE_PERF("Timer Test");
	TIMER_CREATE_PERF("TimerPerf Test");

	return ret;
}

// ---------------------------------------------
void Application::PrepareUpdate()
{
	//TMP, TESTING TIMERS
	TIMER_START_PERF("TimerPerf Test");
	TIMER_START_PERF("__PerfTimer");
	TIMER_READ_MS("__PerfTimer");
	TIMER_READ_MS_MAX("TimerPerf Test");

	TIMER_START_PERF("Timer Test");
	TIMER_START("__Timer");
	TIMER_READ_MS("__Timer");
	TIMER_READ_MS_MAX("Timer Test");
	//////////
	TIMER_START_PERF("App PreUpdate");
	frameCount++;

	//Time managing
	Time.dt = ms_timer.ReadMs() / 1000.0f;
	if (Time.PlayMode && Time.Pause == false)
	{
		Time.gdt = Time.dt / Time.gdtModifier;
		Time.GameRuntime += Time.dt;
	}
	else
	{
		Time.gdt = 0.0f;
	}
	Time.AppRuntime = totalTimer.Read() / 1000.0f;
	//

	ms_timer.Start();

	for (int n = 0; n < EDITOR_FRAME_SAMPLES - 1; n++)
	{
		ms_frame[n] = ms_frame[n + 1];
	}
	ms_frame[EDITOR_FRAME_SAMPLES - 1] = Time.dt;

	float tmp = FPS_Timer.Read();
	if (FPS_Timer.Read() > 1000.0f)
	{
		for (int n = 0; n < EDITOR_FRAME_SAMPLES - 1; n++)
		{
			framerate[n] = framerate[n + 1];
		}
		framerate[EDITOR_FRAME_SAMPLES-1] = frameCount;
		frameCount = 0;
		FPS_Timer.Start();
	}
	
	if (maxFPS != previous_maxFPS)
	{
		if (maxFPS < 5)
		{
			FrameTime = -1.0f;
		}
		else
		{
			FrameTime = 1000.0f / maxFPS;
		}
		previous_maxFPS = maxFPS;
	}
}

// ---------------------------------------------
void Application::FinishUpdate()
{
}

// Call PreUpdate, Update and PostUpdate on all modules
update_status Application::Update()
{
	update_status ret = UPDATE_CONTINUE;
	PrepareUpdate();
	
	std::vector<Module*>::iterator item = list_modules.begin();
	
	while(item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		if ((*item)->IsEnabled())
		{
			ret = (*item)->PreUpdate();
		}
		item++;
	}
	TIMER_READ_MS("App PreUpdate");
	TIMER_START_PERF("App Update");
	item = list_modules.begin();

	while(item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		if ((*item)->IsEnabled())
		{
			ret = (*item)->Update();
		}
		item++;
	}
	TIMER_READ_MS("App Update");
	item = list_modules.begin();
	TIMER_START_PERF("App PostUpdate");
	while(item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		if ((*item)->IsEnabled())
		{
			ret = (*item)->PostUpdate();
		}
		item++;
	}
	TIMER_READ_MS("App PostUpdate");
	FinishUpdate();

	if (FrameTime > 0.0001f)
	{
		while (ms_timer.ReadMs() < FrameTime)
		{
		}
	}
	return ret;
}

bool Application::CleanUp()
{
	gameRunning = false;

	bool ret = true;
	std::vector<Module*>::reverse_iterator item = list_modules.rbegin();

	while(item != list_modules.rend() && ret == true)
	{
		ret = (*item)->CleanUp();
		item++;
	}
	RELEASE(timers);

	return ret;
}

void Application::Render(const viewPort& port)
{
	std::vector<Module*>::iterator item = list_modules.begin();

	while (item != list_modules.end())
	{
		if ((*item)->IsEnabled())
		{
			(*item)->Render(port);
		}
		item++;
	}
}


bool Application::OpenBrowser(const char* link)
{
	ShellExecuteA(0, 0, "chrome.exe", link, 0, SW_SHOWMAXIMIZED);

	return true;
}

void Application::Log(char* str)
{
	if (gameRunning == true)
	{
		if (Editor != nullptr && Editor->IsEnabled())
		{
			Editor->Log(str);
		}
	}
}

const char* Application::GetOrganization()
{
	return organisation.data();
}

const char* Application::GetTitle()
{
	return title.data();
}

void Application::OnScreenResize(int width, int heigth)
{
	for (std::vector<Module*>::iterator it = list_modules.begin(); it != list_modules.end(); it++)
	{
		(*it)->OnScreenResize(width, heigth);
	}
}

void Application::AddModule(Module* mod)
{
	list_modules.push_back(mod);
}