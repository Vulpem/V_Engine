#include "Application.h"


#include "Module.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleAudio.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModulePhysics3D.h"
#include "ModuleEditor.h"
#include "ModuleGoManager.h"
#include "ModuleScene.h"
#include "ModuleFileSystem.h"
#include "ModuleImporter.h"

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
	Editor = new ModuleEditor(this);
	GO = new ModuleGoManager(this);
	importer = new ModuleImporter(this);

	scene = new ModuleScene(this);

	// The order of calls is very important!
	// Modules will Init() Start() and Update in this order
	// They will CleanUp() in reverse order

	// Main Modules
	AddModule(window);
	AddModule(input);
	AddModule(Editor);
	AddModule(camera);
	AddModule(fs);
	AddModule(audio);
	AddModule(physics);
	AddModule(importer);
	AddModule(GO);
	
	// Scenes
	AddModule(scene);


	// Renderer last!
	AddModule(renderer3D);

	for (int n = 0; n < EDITOR_FRAME_SAMPLES; n++)
	{
		ms_frame[n] = 0;
		framerate[n] = 0;
	}
	FrameTime = -1.0f;
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

	return ret;
}

// ---------------------------------------------
void Application::PrepareUpdate()
{
	frameCount++;
	dt = (float)ms_timer.Read() / 1000.0f;
	ms_timer.Start();

	for (int n = 0; n < EDITOR_FRAME_SAMPLES - 1; n++)
	{
		ms_frame[n] = ms_frame[n + 1];
	}
	ms_frame[EDITOR_FRAME_SAMPLES - 1] = dt;

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
			ret = (*item)->PreUpdate(dt);
		}
		item++;
	}

	item = list_modules.begin();

	while(item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		if ((*item)->IsEnabled())
		{
			ret = (*item)->Update(dt);
		}
		item++;
	}

	item = list_modules.begin();

	while(item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		if ((*item)->IsEnabled())
		{
			ret = (*item)->PostUpdate(dt);
		}
		item++;
	}

	FinishUpdate();

	if (FrameTime > 0.0001f)
	{
		while (ms_timer.Read() < FrameTime)
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
	return ret;
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
	return organisation.GetString();
}

const char* Application::GetTitle()
{
	return title.GetString();
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