#include "Globals.h"
#include "Application.h"

#include "ModuleUI.h"
#include "ModuleWindow.h"



#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_sdl_gl3.h"
#include "OpenGL.h"



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
	bool ret = true;

	LOG("Init editor gui with imgui lib version %s", ImGui::GetVersion());

	ImGui_ImplSdlGL3_Init(App->window->GetWindow());
		
	return ret;
}

bool ModuleGUI::Start()
{
	ImGui_ImplSdlGL3_NewFrame(App->window->GetWindow());

	
	tmpInput = new char[64];
	strcpy(tmpInput, "InputTextHere");

	return true;
}

// Called every draw update
update_status ModuleGUI::PreUpdate(float dt)
{
	ImGui_ImplSdlGL3_NewFrame(App->window->GetWindow());
	ImGuiIO& io = ImGui::GetIO();
	capture_keyboard = io.WantCaptureKeyboard;
	capture_mouse = io.WantCaptureMouse;

	ImGui::Button("TestButton", ImVec2(100, 50));
	ImGui::LabelText("label", "cameraX %i", App->camera->Position.x);
	ImGui::LabelText("label", "cameraY %i", App->camera->Position.y);
	ImGui::LabelText("label", "cameraZ %i", App->camera->Position.z);
	ImGui::InputText("input text", tmpInput, 60);


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
	delete[] tmpInput;
	return true;
}

void ModuleGUI::HandleInput(SDL_Event* event)
{
	ImGui_ImplSdlGL3_ProcessEvent(event);
}