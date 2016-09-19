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

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Quit"))
			{
				ret = UPDATE_STOP;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Documentation"))
		{
			if (ImGui::MenuItem("MathGeoLib"))
			{
				App->OpenBrowser("http://clb.demon.fi/MathGeoLib/nightly/reference.html");
			}
			if (ImGui::MenuItem("ImGui"))
			{
				App->OpenBrowser("https://github.com/ocornut/imgui");
			}
			if (ImGui::MenuItem("Bullet"))
			{
				App->OpenBrowser("http://bulletphysics.org/Bullet/BulletFull/annotated.html");
			}
			if (ImGui::MenuItem("SDL"))
			{
				App->OpenBrowser("https://wiki.libsdl.org/APIByCategory");
			}
					
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (ImGui::Begin("Camera"))
	{
		ImGui::InputFloat("X", &App->camera->Position.x);
		ImGui::InputFloat("Y", &App->camera->Position.y);
		ImGui::InputFloat("Z", &App->camera->Position.z);

		ImGui::End();
	}

	ImGui::Button("TestButton", ImVec2(100, 50));
	if (ImGui::Button("Quit", ImVec2(75, 75)))
	{
		ret = UPDATE_STOP;
	}
	ImGui::LabelText("label", "MouseX: %i", App->input->GetMouseX());
	ImGui::SameLine();
	ImGui::LabelText("label", "MouseY: %i", App->input->GetMouseY());

	ImGui::LabelText("label", "CameraX: %f", App->camera->Position.x);
	ImGui::LabelText("label", "CameraY: %f", App->camera->Position.y);
	ImGui::LabelText("label", "CameraZ: %f", App->camera->Position.z);
	ImGui::LabelText("label", "CameraRefX: %i", App->camera->Reference.x);
	ImGui::LabelText("label", "CameraRefY: %i", App->camera->Reference.y);
	ImGui::LabelText("label", "CameraRefZ: %i", App->camera->Reference.z);

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