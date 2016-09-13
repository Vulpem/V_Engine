#include "Globals.h"
#include "Application.h"

#include "ModuleUI.h"
#include"imGUI\imgui.h"

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

	SDL_GL_
	io.
	
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