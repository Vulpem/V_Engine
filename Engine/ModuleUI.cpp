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
#include "ModuleImportGeometry.h"


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
	camRefX = camRefY = camRefZ = 0.0f;

	strcpy(toImport, "FBX/-.fbx");

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
			ImGui::Checkbox("Editor", &IsOpenEditor);
			ImGui::Checkbox("Outliner", &IsOpenOutliner);
			ImGui::Checkbox("Console", &IsOpenConsole);
			ImGui::Checkbox("ImGui TestBox", &IsOpenTestWindow);
			ImGui::Checkbox("CameraReference", &App->camera->renderReference);
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
				ImGui::InputFloat("X##cam", &App->camera->Position.x);
				ImGui::InputFloat("Y##cam", &App->camera->Position.y);
				ImGui::InputFloat("Z##cam", &App->camera->Position.z);
				ImGui::NewLine();
				ImGui::LabelText("##CamRefX", "CameraRefX: %i", App->camera->Reference.x);
				ImGui::LabelText("##CamRefY", "CameraRefY: %i", App->camera->Reference.y);
				ImGui::LabelText("##CamRefZ", "CameraRefZ: %i", App->camera->Reference.z);
				ImGui::NewLine();
				ImGui::InputFloat("Distance to Ref", &App->camera->distanceToRef);


				if (ImGui::BeginPopup("SetCameraRef"))
				{
					ImGui::InputFloat("RefX", &camRefX);
					ImGui::InputFloat("RefY", &camRefY);
					ImGui::InputFloat("RefZ", &camRefZ);
					if (ImGui::Button("Set"))
					{
						App->camera->LookAt(vec3(camRefX, camRefY, camRefZ));
					}

					ImGui::EndPopup();
				}

				if (ImGui::Button("SetCameraReference"))
				{
					ImGui::OpenPopup("SetCameraRef");
				}

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
								ImGui::InputFloat(tmp, &App->renderer3D->lights[nLight].position.x);
								sprintf(tmp, "Y##light_%i", nLight);
								ImGui::InputFloat(tmp, &App->renderer3D->lights[nLight].position.y);
								sprintf(tmp, "Z##light_%i", nLight);
								ImGui::InputFloat(tmp, &App->renderer3D->lights[nLight].position.z);
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
		ImGui::Begin("Outliner", &IsOpenOutliner, ImVec2(500, 300), 1.0f, 0);
		if (ImGui::CollapsingHeader("Load Geometry"))
		{
			ImGui::InputText("Load:", toImport, 256);
			if (ImGui::Button("Import"))
			{
				Node* import = App->importGeometry->LoadFBX(toImport);
				if (import != NULL)
				{
					geometries.push_back(App->importGeometry->LoadFBX(toImport));
					importResult = "Import successful!";
				}
				else
				{
					importResult = "Error importing.";
				}


			}
			ImGui::Text(importResult.GetString());
		}

		if (ImGui::TreeNode("Scene"))
		{
			std::vector<Node*>::iterator node = App->importGeometry->geometryNodes.begin();
			while (node != App->importGeometry->geometryNodes.end())
			{
				std::vector<Node*>::iterator childNodes = (*node)->childs.begin();
				while (childNodes != (*node)->childs.end())
				{
					SceneTreeNodes((*childNodes));
					childNodes++;
				}				
				node++;
			}
			ImGui::TreePop();
		}
		ImGui::End();
	}
#pragma endregion

#pragma region Attributes window
	if (IsOpenAttributes)
	{
		ImGui::Begin("Attribute Editor", &IsOpenAttributes);
		if (selectedGeometry)
		{
			ImGui::Text(selectedGeometry->name.GetString());
			ImGui::InputFloat3("Position", selectedPos, 2);
			ImGui::InputFloat3("Rotation", selectedEuler, 2);
			ImGui::InputFloat3("Scale", selectedScale, 2);

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

void ModuleUI::SceneTreeNodes(Node* node)
{
	if (ImGui::TreeNode(node->name.GetString()))
	{
		if (ImGui::IsItemClicked())
		{
			SelectNode(node);
		}
		//math::float3 pos = node->GetPos();
		//ImGui::SameLine();
		//ImGui::Text("X:%.1f, Y:%.1f, Z:%.1f", pos.x, pos.y, pos.z);

		std::vector<Node*>::iterator it = node->childs.begin();
		while (it != node->childs.end())
		{
			SceneTreeNodes((*it));
			it++;
		}
		ImGui::TreePop();
	}
}

void ModuleUI::SelectNode(Node* node)
{
	selectedGeometry = node;
	math::float3 pos, rot, scale;
	math::Quat tmpRot;
	node->transform.Decompose(pos,tmpRot, scale);
	rot = tmpRot.ToEulerXYZ();

	selectedPos[0] = pos.x;
	selectedPos[1] = pos.y;
	selectedPos[2] = pos.z;

	selectedEuler[0] = rot.x;
	selectedEuler[1] = rot.y;
	selectedEuler[2] = rot.z;

	selectedScale[0] = scale.x;
	selectedScale[1] = scale.y;
	selectedScale[2] = scale.z;
}