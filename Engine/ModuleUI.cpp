#include "Globals.h"
#include "Application.h"
#include "ModuleUI.h"

#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleAudio.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModulePhysics3D.h"
#include "ModuleScene.h"
#include "ModuleGOmanager.h"


#include "Imgui/imgui_impl_sdl_gl3.h"
#include "OpenGL.h"

ModuleUI::ModuleUI(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name.create("ModuleUI");
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
	
	testConsoleInput = new char[64];
	strcpy(testConsoleInput, "InputTextHere");

	strcpy(toImport, "FBX/-.fbx");

	selectedGameObject = NULL;

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

	bool tmp = true;

	int screenW, screenH;
	App->window->GetWindowSize(&screenW, &screenH);

#pragma region MenuBar
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Quit"))
			{
				ret = UPDATE_STOP;
			}
			if (ImGui::MenuItem("ClearConsole"))
			{
				ClearConsole();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			ImGui::Checkbox("Outliner", &IsOpenOutliner);
			ImGui::Checkbox("Editor", &IsOpenEditor);
			ImGui::Checkbox("Attribute Editor", &IsOpenAttributes);
			ImGui::Checkbox("Console", &IsOpenConsole);
			ImGui::Checkbox("ImGui TestBox", &IsOpenTestWindow);		
			ImGui::Checkbox("CameraReference", &App->camera->renderReference);
			ImGui::Checkbox("InGame Plane", &showPlane);
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
#pragma endregion

#pragma region Editor
	if (IsOpenEditor)
	{
		ImGui::SetNextWindowPos(ImVec2(screenW - 330, 530));
		ImGui::SetNextWindowSize(ImVec2(330, screenH -530));

		ImGui::Begin("Editor", &IsOpenEditor, ImVec2(500, 300), 1.0f, 0);

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
				ImGui::Text("Position");
				ImGui::DragFloat("X##cam", &App->camera->Position.x);
				ImGui::DragFloat("Y##cam", &App->camera->Position.y);
				ImGui::DragFloat("Z##cam", &App->camera->Position.z);
				ImGui::NewLine();
				ImGui::Text("Reference");
				if (ImGui::DragFloat3("##CamReference", camRef, 1.0f))
				{
					App->camera->LookAt(vec3(camRef[0], camRef[1], camRef[2]));
				}
				ImGui::NewLine();
				ImGui::Text("Distance to reference");	
				if(ImGui::DragFloat("##Distance to reference", &App->camera->distanceToRef, 1.0f, 1.0f))
				{
					App->camera->UpdateView();
				}
				ImGui::Text("Camera speed");
				ImGui::DragFloat("##camSpeed", &App->camera->camSpeed, 0.1f);
				ImGui::Text("Sprint speed multiplier");
				ImGui::DragFloat("##camsprint", &App->camera->camSprintMultiplier, 0.1f);
								
			}

			if (ImGui::CollapsingHeader("Render"))
			{
				if (ImGui::TreeNode("Lights"))
				{
					for (int nLight = 0; nLight < MAX_LIGHTS; nLight++)
					{
						char lightName[46];
						sprintf(lightName, "Light %i", nLight);
						bool on = App->renderer3D->lights[nLight].on;
						ImGui::Checkbox(lightName, &on);

						if (on != App->renderer3D->lights[nLight].on)
						{
							App->renderer3D->lights[nLight].Active(on);
						}
						if (App->renderer3D->lights[nLight].on == true)
						{

							sprintf(lightName, "Expand##Light_%i", nLight);
							ImGui::SameLine();
							if (ImGui::TreeNode(lightName))
							{
								char tmp[46];
								sprintf(tmp, "X##light_%i", nLight);
								ImGui::DragFloat(tmp, &App->renderer3D->lights[nLight].position.x, 1.0f);
								sprintf(tmp, "Y##light_%i", nLight);
								ImGui::DragFloat(tmp, &App->renderer3D->lights[nLight].position.y, 1.0f);
								sprintf(tmp, "Z##light_%i", nLight);
								ImGui::DragFloat(tmp, &App->renderer3D->lights[nLight].position.z, 1.0f);
								ImGui::TreePop();
							}
						}
					}
					ImGui::TreePop();
				}
			}

			if (ImGui::CollapsingHeader("Tests"))
			{
				ImGui::InputText("##consoleTest", testConsoleInput, 60);
				ImGui::SameLine();
				if (ImGui::Button("TestConsole"))
				{
					LOG(testConsoleInput);
				}

			}
			ImGui::End();
	}
#pragma endregion

#pragma region Console
	if (IsOpenConsole)
	{
		ImGui::SetNextWindowPos(ImVec2(0.0f, screenH - 200.0f));
		ImGui::SetNextWindowSize(ImVec2(screenW - 330, 200));

		ImGui::Begin("Console", &tmp, ImVec2(500, 300), 1.0f, 0);

		ImColor col = ImColor(0.6f, 0.6f, 1.0f, 1.0f);
		ImGui::PushStyleColor(0, col);

		ImGui::TextUnformatted(buffer.begin());
		ImGui::PopStyleColor();

		if (scrollToBottom)
			ImGui::SetScrollHere(1.0f);

		scrollToBottom = false;

		ImGui::End();
	}
#pragma endregion

#pragma region outliner
	if (IsOpenOutliner)
	{
		ImGui::SetNextWindowPos(ImVec2(0.0f, 20.0f));
		ImGui::SetNextWindowSize(ImVec2(300, screenH - 220));

		ImGui::Begin("Outliner", &IsOpenOutliner, ImVec2(500, 300), 1.0f, 0);
		if (ImGui::CollapsingHeader("Load Geometry"))
		{
			ImGui::InputText("Load:", toImport, 256);
			if (ImGui::Button("Import"))
			{
				GameObject* import = App->GO->LoadFBX(toImport);
				if (import != NULL)
				{
					geometries.push_back(App->GO->LoadFBX(toImport));
					importResult = "Import successful!";
				}
				else
				{
					importResult = "Error importing.";
				}


			}
			ImGui::Text(importResult.GetString());
		}

		//if (ImGui::TreeNode("Scene"))
		//{
			std::vector<GameObject*>::iterator node = App->GO->root->childs.begin();
			while (node != App->GO->root->childs.end())
			{
				std::vector<GameObject*>::iterator childNodes = (*node)->childs.begin();
				while (childNodes != (*node)->childs.end())
				{
					SceneTreeGameObject((*childNodes));
					childNodes++;
				}				
				node++;
			}
		//	ImGui::TreePop();
		//}
		ImGui::End();
	}
#pragma endregion

#pragma region Attributes window
	if (IsOpenAttributes)
	{
		ImGui::SetNextWindowPos(ImVec2(screenW - 330, 20.0f));
		ImGui::SetNextWindowSize(ImVec2(330, 510));
		ImGui::Begin("Attribute Editor", &IsOpenAttributes);
		if (selectedGameObject)
		{
			ImGui::InputText("Name", selectedGameObject->name, NAME_MAX_LEN);
			if (ImGui::DragFloat3("Position", selectedPos, 1.0f))
			{
				selectedGameObject->SetPos(selectedPos[0], selectedPos[1], selectedPos[2]);
			}
			if (ImGui::DragFloat3("Rotation", selectedEuler, 1.0f, 0.0f, 360.0f))
			{
				selectedGameObject->SetRot(selectedEuler[0], selectedEuler[1], selectedEuler[2]);
			}
			if (ImGui::DragFloat3("Scale", selectedScale, 0.01f, 0.1f))
			{
				selectedGameObject->SetScale(selectedScale[0], selectedScale[1], selectedScale[2]);
			}
			ImGui::NewLine();
			ImGui::Text("Danger Zone:");
			if (ImGui::Button("Delete"))
			{
				App->GO->DeleteGameObject(selectedGameObject);
				selectedGameObject = NULL;
			}

		}
		ImGui::End();
	}
#pragma endregion

	return ret;
}

update_status ModuleUI::Update(float dt)
{
	if (IsOpenTestWindow)
	{
		ImGui::ShowTestWindow(&IsOpenTestWindow);
	}
	if (showPlane)
	{
		P_Plane p(0, 0, 0, 1);
		p.axis = true;
		p.Render();
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
	ClearConsole();

	ImGui_ImplSdlGL3_Shutdown();

	delete[] testConsoleInput;
	return true;
}

void ModuleUI::HandleInput(SDL_Event* event)
{
	ImGui_ImplSdlGL3_ProcessEvent(event);
}

void ModuleUI::Log(const char* input)
{
	buffer.append(input);
	scrollToBottom = true;
}

void ModuleUI::ClearConsole()
{
	buffer.clear();
	scrollToBottom = true;
}

void ModuleUI::SceneTreeGameObject(GameObject* node)
{
	if (ImGui::TreeNode(node->name))
	{
		if (ImGui::IsItemClicked())
		{
			SelectGameObject(node);
		}

		std::vector<GameObject*>::iterator it = node->childs.begin();
		while (it != node->childs.end())
		{
			SceneTreeGameObject((*it));
			it++;
		}
		ImGui::TreePop();
	}
}

void ModuleUI::SelectGameObject(GameObject* node)
{
	if (selectedGameObject)
	{
		selectedGameObject->Unselect();
	}
	if (node)
	{
		node->Select();
	}
	selectedGameObject = node;
	if (selectedGameObject)
	{
		math::float3 pos, rot, scale;

		pos = node->GetPos();
		selectedPos[0] = pos.x;
		selectedPos[1] = pos.y;
		selectedPos[2] = pos.z;

		rot = node->GetRot();
		selectedEuler[0] = rot.x;
		selectedEuler[1] = rot.y;
		selectedEuler[2] = rot.z;

		scale = node->GetScale();
		selectedScale[0] = scale.x;
		selectedScale[1] = scale.y;
		selectedScale[2] = scale.z;

		App->camera->LookAt(vec3(pos.x, pos.y, pos.z));
	}
}