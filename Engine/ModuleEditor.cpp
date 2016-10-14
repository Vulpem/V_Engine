#include "Globals.h"
#include "Application.h"
#include "ModuleEditor.h"

#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleAudio.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModulePhysics3D.h"
#include "ModuleScene.h"
#include "ModuleGOmanager.h"

#include "AllComponents.h"

#include "Imgui/imgui_impl_sdl_gl3.h"
#include "OpenGL.h"

ModuleEditor::ModuleEditor(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name.create("ModuleEditor");
}

// Destructor
ModuleEditor::~ModuleEditor()
{

}

// Called before render is available
bool ModuleEditor::Init()
{
	bool ret = true;

	LOG("Init editor gui with imgui lib version %s", ImGui::GetVersion());

	ImGui_ImplSdlGL3_Init(App->window->GetWindow());
		
	return ret;
}

bool ModuleEditor::Start()
{
	ImGui_ImplSdlGL3_NewFrame(App->window->GetWindow());
	
	testConsoleInput = new char[64];
	strcpy(testConsoleInput, "InputTextHere");

	strcpy(toImport, "");

	selectedGameObject = NULL;

	return true;
}

// Called every draw update
update_status ModuleEditor::PreUpdate(float dt)
{
	update_status ret = UPDATE_CONTINUE;
	ImGui_ImplSdlGL3_NewFrame(App->window->GetWindow());
	ImGuiIO& io = ImGui::GetIO();
	capture_keyboard = io.WantCaptureKeyboard;
	capture_mouse = io.WantCaptureMouse;

	bool tmp = true;

	int screenW, screenH;
	App->window->GetWindowSize(&screenW, &screenH);

	ImGuiStyle style = ImGui::GetStyle();
	style.Alpha = 0.9f;


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
			ImGui::Checkbox("InGame Plane", &showPlane);
			ImGui::Checkbox("Reference Axis", &showAxis);
			
			if (ImGui::Checkbox("Render Normals", &renderNormals))
			{
				SelectGameObject(selectedGameObject);
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
#pragma endregion

#pragma region Editor
	if (IsOpenEditor)
	{
		ImGui::SetNextWindowPos(ImVec2(screenW - 330, 530));
		ImGui::SetNextWindowSize(ImVec2(330, screenH -530));

		ImGui::Begin("Editor", &IsOpenEditor, ImVec2(500, 300), 0.8f);

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

		ImGui::Begin("Console", &tmp, ImVec2(500, 300), 0.8f);

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

		ImGui::Begin("Outliner", &IsOpenOutliner, ImVec2(500, 300), 0.8f);
		if (ImGui::CollapsingHeader("Load Geometry"))
		{
			ImGui::InputText("Load:", toImport, 256);
			if (ImGui::Button("Import"))
			{
				std::vector<GameObject*> import = App->GO->LoadGO(toImport);
				if (import.empty() == false)
				{
					importResult = "Import successful!";
				}
				else
				{
					importResult = "Error importing.";
				}


			}
			ImGui::Text(importResult.GetString());
		}

			std::vector<GameObject*>::iterator node = App->GO->root->childs.begin();
			while (node != App->GO->root->childs.end())
			{
					//std::vector<GameObject*>::iterator childNodes = (*node)->childs.begin();
					//while (childNodes != (*node)->childs.end())
					//{
					//	SceneTreeGameObject((*childNodes));
					//	childNodes++;
					//}
				SceneTreeGameObject((*node));
				node++;
			}

		ImGui::End();
	}
#pragma endregion

#pragma region Attributes window
	if (IsOpenAttributes)
	{
		ImGui::SetNextWindowPos(ImVec2(screenW - 330, 20.0f));
		ImGui::SetNextWindowSize(ImVec2(330, 510));
		ImGui::Begin("Attribute Editor", &IsOpenAttributes, 0.8f);
		if (selectedGameObject)
		{
			bool isActive = selectedGameObject->IsActive();
			ImGui::Checkbox("", &isActive);
			if (isActive != selectedGameObject->IsActive())
			{
				selectedGameObject->SetActive(isActive);
			}
			ImGui::SameLine();
			ImGui::Text("Name:");
			ImGui::SameLine();
			ImGui::InputText("##Name", selectedGameObject->name, NAME_MAX_LEN);
			selectedGameObject->DrawOnEditor();
			ImGui::Separator();
			if (ImGui::Button("Look at"))
			{
				Transform* trans = *selectedGameObject->GetComponent<Transform>().begin();

				App->camera->LookAt(vec3(trans->GetPos().x, trans->GetPos().y, trans->GetPos().z));
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


	if (ImGui::IsMouseHoveringAnyWindow())
	{
		capture_mouse = true;
	}

	return ret;
}

update_status ModuleEditor::Update(float dt)
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

#pragma region Axis

	if (showAxis == true)
	{
		math::float3 axisPos(App->camera->Position.x, App->camera->Position.y, App->camera->Position.z);
		math::float3 front(App->camera->Z.x, App->camera->Z.y, App->camera->Z.z);
		math::float3 right(App->camera->X.x, App->camera->X.y, App->camera->X.z);
		math::float3 up(App->camera->Y.x, App->camera->Y.y, App->camera->Y.z);
		axisPos = axisPos - (front)+right * 0.35f - up*0.2f;

		glDisable(GL_LIGHTING);
		// Draw Axis Grid
		glLineWidth(1.0f);

		glBegin(GL_LINES);

		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

		glVertex3f(axisPos.x, axisPos.y, axisPos.z);					glVertex3f(axisPos.x + 0.1f, axisPos.y, axisPos.z);
		glVertex3f(axisPos.x + 0.1f, axisPos.y + 0.01f, axisPos.z);		glVertex3f(axisPos.x + 0.11f, axisPos.y - 0.01f, axisPos.z);
		glVertex3f(axisPos.x + 0.11f, axisPos.y + 0.01f, axisPos.z);	glVertex3f(axisPos.x + 0.10f, axisPos.y - 0.01f, axisPos.z);

		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

		glVertex3f(axisPos.x, axisPos.y, axisPos.z);					glVertex3f(axisPos.x, axisPos.y + 0.1f, axisPos.z);
		glVertex3f(axisPos.x - 0.005f, axisPos.y + 0.125f, axisPos.z);	glVertex3f(axisPos.x, axisPos.y + 0.115f, axisPos.z);
		glVertex3f(axisPos.x + 0.005f, axisPos.y + 0.125f, axisPos.z);	glVertex3f(axisPos.x, axisPos.y + 0.115f, axisPos.z);
		glVertex3f(axisPos.x, axisPos.y + 0.115f, axisPos.z);				glVertex3f(axisPos.x, axisPos.y + 0.105f, axisPos.z);

		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);

		glVertex3f(axisPos.x, axisPos.y, axisPos.z);					glVertex3f(axisPos.x, axisPos.y, axisPos.z + 0.1f);

		glVertex3f(axisPos.x - 0.005f, axisPos.y + 0.01, axisPos.z + 0.105f);			glVertex3f(axisPos.x + 0.005f, axisPos.y + 0.01f, axisPos.z + 0.105f);
		glVertex3f(axisPos.x + 0.005f, axisPos.y + 0.01f, axisPos.z + 0.105f);	glVertex3f(axisPos.x - 0.005f, axisPos.y - 0.01f, axisPos.z + 0.105f);
		glVertex3f(axisPos.x - 0.005f, axisPos.y - 0.01f, axisPos.z + 0.105f);		glVertex3f(axisPos.x + 0.005f, axisPos.y - 0.01f, axisPos.z + 0.105f);

		glEnd();

		glLineWidth(1.0f);
		glEnable(GL_LIGHTING);
	}

#pragma endregion

	return UPDATE_CONTINUE;
}

update_status ModuleEditor::PostUpdate(float dt)
{
	ImGui::Render();
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleEditor::CleanUp()
{
	ClearConsole();

	ImGui_ImplSdlGL3_Shutdown();

	delete[] testConsoleInput;
	return true;
}

void ModuleEditor::HandleInput(SDL_Event* event)
{
	ImGui_ImplSdlGL3_ProcessEvent(event);
}

void ModuleEditor::Log(const char* input)
{
	buffer.append(input);
	scrollToBottom = true;
}

void ModuleEditor::ClearConsole()
{
	buffer.clear();
	scrollToBottom = true;
}

void ModuleEditor::SceneTreeGameObject(GameObject* node)
{
	if (node->HiddenFromOutliner() == false)
	{
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (selectedGameObject == node)
		{
			node_flags += ImGuiTreeNodeFlags_Selected;
		}
		if (node->childs.empty())
		{
			node_flags += ImGuiTreeNodeFlags_Leaf;
		}

		if (ImGui::TreeNodeEx(node->name, node_flags))
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
}

void ModuleEditor::SelectGameObject(GameObject* node)
{
	if (selectedGameObject)
	{
		selectedGameObject->Unselect();
	}
	if (node)
	{
		node->Select(renderNormals);
	}
	selectedGameObject = node;
}