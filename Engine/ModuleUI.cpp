#include "Globals.h"
#include "Application.h"

#include "ModuleUI.h"
#include "ModuleWindow.h"



#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_sdl_gl3.h"
#include "OpenGL.h"



ModuleUI::ModuleUI(Application* app, bool start_enabled) : Module(app, start_enabled)
{

}

// Destructor
ModuleUI::~ModuleUI()
{

}

// Called before render is available
bool ModuleUI::Init()
{
	bool ret = true;

	LOG("Init editor gui with imgui lib version %s", ImGui::GetVersion());

	ImGui_ImplSdlGL3_Init(App->window->GetWindow());
		
	return ret;
}

bool ModuleUI::Start()
{
	ImGui_ImplSdlGL3_NewFrame(App->window->GetWindow());

	
	tmpInput = new char[64];
	strcpy(tmpInput, "InputTextHere");

	return true;
}

// Called every draw update
update_status ModuleUI::PreUpdate(float dt)
{
	update_status ret = UPDATE_CONTINUE;
	ImGui_ImplSdlGL3_NewFrame(App->window->GetWindow());
	ImGuiIO& io = ImGui::GetIO();
	capture_keyboard = io.WantCaptureKeyboard;
	capture_mouse = io.WantCaptureMouse;

	ImGui::Button("TestButton", ImVec2(100, 50));
	if (ImGui::Button("Quit", ImVec2(75, 75)))
	{
		ret = UPDATE_STOP;
	}
	ImGui::LabelText("label", "MouseX: %i", App->input->GetMouseX());
	ImGui::LabelText("label", "MouseY: %i", App->input->GetMouseY());
	ImGui::InputText("input text", tmpInput, 60);

	return ret;
}

update_status ModuleUI::Update(float dt)
{
	ImGui::ShowTestWindow(&testWindowOpen);


	return UPDATE_CONTINUE;
}

update_status ModuleUI::PostUpdate(float dt)
{
	ImGui::Render();
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleUI::CleanUp()
{
	delete[] tmpInput;
	return true;
}

void ModuleUI::HandleInput(SDL_Event* event)
{
	ImGui_ImplSdlGL3_ProcessEvent(event);
}