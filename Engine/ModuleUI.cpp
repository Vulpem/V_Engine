#include "Globals.h"
#include "Application.h"
#include "ModuleUI.h"

#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleAudio.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModulePhysics3D.h"
#include "ModuleTests.h"
#include "ModuleScene.h"



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

	console = new UI_Console();

	
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

	console->Draw(&consoleOpen);

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
		if (ImGui::BeginMenu("View"))
		{
			ImGui::Checkbox("Editor", &editorOpen);
			ImGui::Checkbox("Console", &consoleOpen);
			ImGui::Checkbox("ImGui TestBox", &testWindowOpen);
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

	bool tmp = true;
	if (editorOpen == true)
	{
		if (ImGui::Begin("Editor", &tmp, ImVec2(500, 300), 1.0f, 0))
		{

			if (ImGui::CollapsingHeader("Application"))
			{
				ImGui::InputInt("Max Framerate:", &App->maxFPS, 15);
				char tmp[256];
				sprintf(tmp, "Framerate: %i", int(App->framerate[EDITOR_FRAME_SAMPLES - 1]));
				ImGui::PlotHistogram("##Framerate:", App->framerate, EDITOR_FRAME_SAMPLES - 1, 0, tmp, 0.0f, 100.0f, ImVec2(310, 100));

				char tmp2[256];
				sprintf(tmp2, "Ms: %i", int(App->ms_frame[EDITOR_FRAME_SAMPLES - 1] * 1000));
				ImGui::PlotHistogram("##ms", App->ms_frame, EDITOR_FRAME_SAMPLES - 1, 0, tmp2, 0.0f, 0.07f, ImVec2(310, 100));
			}

			if (ImGui::CollapsingHeader("Input"))
			{
				ImGui::LabelText("label", "MouseX: %i", App->input->GetMouseX());
				ImGui::LabelText("label", "MouseY: %i", App->input->GetMouseY());
			}

			if (ImGui::CollapsingHeader("Camera"))
			{
				ImGui::InputFloat("X", &App->camera->Position.x);
				ImGui::InputFloat("Y", &App->camera->Position.y);
				ImGui::InputFloat("Z", &App->camera->Position.z);
				ImGui::LabelText("##CamRefX", "CameraRefX: %i", App->camera->Reference.x);
				ImGui::LabelText("##CamRefY", "CameraRefY: %i", App->camera->Reference.y);
				ImGui::LabelText("##CamRefZ", "CameraRefZ: %i", App->camera->Reference.z);
			}
			
			ImGui::InputText("input text", tmpInput, 60);
			LOG(tmpInput);

			ImGui::End();
		}
	}
	return ret;
}

update_status ModuleUI::Update(float dt)
{
	if (testWindowOpen)
	{
		ImGui::ShowTestWindow(&testWindowOpen);
	}


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
	if (console)
	{
		delete console;
		console = NULL;
	}

	ImGui_ImplSdlGL3_Shutdown();

	delete[] tmpInput;
	return true;
}

void ModuleUI::HandleInput(SDL_Event* event)
{
	ImGui_ImplSdlGL3_ProcessEvent(event);
}

void ModuleUI::Log(const char* input)
{
	if (console != NULL && console->active)
	{
		console->AddLog(input);
	}
}