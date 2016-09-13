#include "Globals.h"
#include "Application.h"

#include "ModuleWindow.h"

#include "ModuleUI.h"


ModuleGUI::ModuleGUI(Application* app, bool start_enabled) : Module(app, start_enabled)
{

}

// Destructor
ModuleGUI::~ModuleGUI()
{

}

// Called before render is available
bool ModuleGUI::Init()
{
	LOG("Init UI");
	bool ret = true;

	ImGuiIO& io = ImGui::GetIO();
	
	io.DisplaySize.x = App->window->screen_surface->w;
	io.DisplaySize.y = App->window->screen_surface->h;

	//io.Fonts->
	
	//guiIO.Fonts->GetTexDataAsRGBA32();

		
	return ret;
}

// Called every draw update
update_status ModuleGUI::PreUpdate(float dt)
{

	ImGui::NewFrame();

	if (false)
	{
		return UPDATE_STOP;
	}

	return UPDATE_CONTINUE;
}

update_status ModuleGUI::Update(float dt)
{
	ImGui::ShowTestWindow(&testWindowOpen);
	return UPDATE_CONTINUE;
}

update_status ModuleGUI::PostUpdate(float dt)
{
	ImGui::Render();
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleGUI::CleanUp()
{

	return true;
}