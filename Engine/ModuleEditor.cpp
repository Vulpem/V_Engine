#include "Globals.h"
#include "Application.h"
#include "ModuleEditor.h"

#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleAudio.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModulePhysics3D.h"
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
//Linking ImGUI and the window
ImGui_ImplSdlGL3_Init(App->window->GetWindow());

return ret;
}

bool ModuleEditor::Start()
{
	ImGui_ImplSdlGL3_NewFrame(App->window->GetWindow());

	//Initializing the strings used to test the editor
	strcpy(testConsoleInput, "InputTextHere");
	strcpy(toImport, "");

	selectedGameObject = nullptr;

	App->renderer3D->FindViewPort(0)->active = false;

	singleViewPort = App->renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->camera->GetDefaultCam());
	multipleViewPorts[0] = App->renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->camera->GetDefaultCam());
	multipleViewPorts[1] = App->renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->camera->GetTopCam());
	multipleViewPorts[2] = App->renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->camera->GetRightCam());
	multipleViewPorts[3] = App->renderer3D->AddViewPort(float2(0, 0), float2(100, 100), App->camera->GetFrontCam());

	OnScreenResize(App->window->GetWindowSize().x, App->window->GetWindowSize().y);
	SwitchViewPorts();

	return true;
}

// Called every draw update
update_status ModuleEditor::PreUpdate(float dt)
{
	update_status ret = UPDATE_CONTINUE;
	ImGui_ImplSdlGL3_NewFrame(App->window->GetWindow());

	ImGuiIO IO = ImGui::GetIO();
	App->input->ignoreMouse = IO.WantCaptureMouse;

	if (IO.WantCaptureKeyboard || IO.WantTextInput)
	{
		App->input->ignoreKeyboard = true;
	}
	else
	{
		App->input->ignoreKeyboard = false;
	}

	return ret;
}

update_status ModuleEditor::Update(float dt)
{
	update_status ret = UPDATE_CONTINUE;

	if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_DOWN || App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		viewPort* port = App->renderer3D->HoveringViewPort();
		if (port != nullptr)
		{
			App->camera->SetMovingCamera(port->camera);
		}
	}

	SelectByViewPort();

	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
	{
		multipleViews = !multipleViews;
		SwitchViewPorts();
	}


	if (IsOpenTestWindow)
	{
		ImGui::ShowTestWindow(&IsOpenTestWindow);
	}

	ret = MenuBar();
	Editor();
	Console();
	Outliner();
	AttributeWindow();

	return ret;
}

update_status ModuleEditor::PostUpdate(float dt)
{
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleEditor::CleanUp()
{
	ClearConsole();

	ImGui_ImplSdlGL3_Shutdown();

	return true;
}

void ModuleEditor::Render(const viewPort & port)
{
	//Here we put the UI we'll draw for each viewport, since Render is called one time for each port that's active
	ViewPortUI(port);

	App->renderer3D->DrawLine(selectRay.a, selectRay.b, float4(1.0f, 1.0f, 1.0f, 1.0f));

	App->renderer3D->DrawLocator(out_pos, float4(0.75f, 0.75f, 0.75f,1));
	App->renderer3D->DrawLine(out_pos, out_pos + out_normal * 2, float4(1, 1, 0, 1));

	if (showPlane)
	{
		P_Plane p(0, 0, 0, 1);
		p.axis = true;
		p.Render();
	}
}

void ModuleEditor::OnScreenResize(int width, int heigth)
{
	screenW = width;
	screenH = heigth;
	viewPortMax.x = screenW - 330;
	viewPortMax.y = screenH - 200;
	viewPortMin.x = 300;
	viewPortMin.y = 20;

	//Setting the single viewPort data
	viewPort* port = App->renderer3D->FindViewPort(singleViewPort);
	port->pos = viewPortMin;
	port->size.x = viewPortMax.x - viewPortMin.x;
	port->size.y = viewPortMax.y - viewPortMin.y;

	//Setting the multiple viewPort data
	float2 size((viewPortMax.x - viewPortMin.x) / 2, (viewPortMax.y - viewPortMin.y) / 2);
	port = App->renderer3D->FindViewPort(multipleViewPorts[0]);
	port->pos = viewPortMin;
	port->size = size;

	port = App->renderer3D->FindViewPort(multipleViewPorts[1]);
	port->pos = viewPortMin;
	port->pos.x += size.x;
	port->size = size;

	port = App->renderer3D->FindViewPort(multipleViewPorts[2]);
	port->pos = viewPortMin;
	port->pos.y += size.y;
	port->size = size;

	port = App->renderer3D->FindViewPort(multipleViewPorts[3]);
	port->pos = viewPortMin;
	port->pos.x += size.x;
	port->pos.y += size.y;
	port->size = size;

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

		if (ImGui::TreeNodeEx(node->GetName(), node_flags))
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

update_status ModuleEditor::MenuBar()
{
	update_status ret = UPDATE_CONTINUE;
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
			if (ImGui::Checkbox("Multiple Views", &multipleViews))
			{
				SwitchViewPorts();
			}
			ImGui::Checkbox("ImGui TestBox", &IsOpenTestWindow);
			ImGui::Checkbox("InGame Plane", &showPlane);
			ImGui::Checkbox("QuadTree", &App->GO->drawQuadTree);
			ImGui::Checkbox("Reference Axis", &showAxis);

			if (ImGui::Checkbox("Render Normals", &renderNormals))
			{
				SelectGameObject(selectedGameObject);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::MenuItem("Empty##CreateEmpty") == true)
			{
				App->GO->CreateEmpty();
			}
			if (ImGui::MenuItem("Camera##CreateEmptyCam") == true)
			{
				App->GO->CreateCamera();
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
	return ret;
}

void ModuleEditor::Editor()
{
		ImGui::SetNextWindowPos(ImVec2(screenW - 330, 530));
		ImGui::SetNextWindowSize(ImVec2(330, screenH - 530));

		ImGui::Begin("Editor", 0, ImVec2(500, 300), 0.8f);

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

		if (ImGui::CollapsingHeader("Camera##CameraModule"))
		{
			ImGui::Text("Position");
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

void ModuleEditor::Console()
{
		ImGui::SetNextWindowPos(ImVec2(0.0f, screenH - 200.0f));
		ImGui::SetNextWindowSize(ImVec2(screenW - 330.0f, 200.0f));

		ImGui::Begin("Console", 0, ImVec2(500, 300), 0.8f);

		ImColor col = ImColor(0.6f, 0.6f, 1.0f, 1.0f);
		ImGui::PushStyleColor(0, col);

		ImGui::TextUnformatted(buffer.begin());
		ImGui::PopStyleColor();

		if (scrollToBottom)
			ImGui::SetScrollHere(1.0f);

		scrollToBottom = false;

		ImGui::End();
}

void ModuleEditor::Outliner()
{
		ImGui::SetNextWindowPos(ImVec2(0.0f, 20.0f));
		ImGui::SetNextWindowSize(ImVec2(300.0f, screenH - 220.0f));

		ImGui::Begin("Outliner", 0, ImVec2(500, 300), 0.8f);

		std::vector<GameObject*>::const_iterator node = App->GO->GetRoot()->childs.begin();
		while (node != App->GO->GetRoot()->childs.end())
		{
			SceneTreeGameObject((*node));
			node++;
		}

		ImGui::End();
}


void ModuleEditor::AttributeWindow()
{
		ImGui::SetNextWindowPos(ImVec2(screenW - 330, 20.0f));
		ImGui::SetNextWindowSize(ImVec2(330, 510));
		ImGui::Begin("Attribute Editor", 0, 0.8f);
		if (selectedGameObject)
		{
			selectedGameObject->DrawOnEditor();
			ImGui::Separator();
			if (selectedGameObject->HasComponent(Component::Type::C_transform))
			{
				if (ImGui::Button("Look at"))
				{
					float3 toLook = selectedGameObject->GetTransform()->GetGlobalPos();
					App->camera->LookAt(float3(toLook.x, toLook.y, toLook.z));
				}
				ImGui::NewLine();
				ImGui::Text("Danger Zone:");
				if (ImGui::Button("Delete"))
				{
					App->GO->DeleteGameObject(selectedGameObject);
					selectedGameObject = nullptr;
				}
			}
		}
		ImGui::End();
}

void ModuleEditor::SwitchViewPorts()
{
	App->renderer3D->FindViewPort(singleViewPort)->active = !multipleViews;
	for (int n = 0; n < 4; n++)
	{
		App->renderer3D->FindViewPort(multipleViewPorts[n])->active = multipleViews;
	}
}

void ModuleEditor::ViewPortUI(const viewPort& port)
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;

	ImGui::SetNextWindowPos(ImVec2(port.pos.x, port.pos.y));
	ImGui::SetNextWindowSize(ImVec2(port.size.x, 0));

	char tmp[256];
	sprintf(tmp, "ViewPortMenu##%i", port.ID);

	ImGui::Begin(tmp, 0, flags);
	if (ImGui::BeginMenuBar())
	{
		sprintf(tmp, "Camera:##ViewPort%i", port.ID);
		if (ImGui::BeginMenu(tmp))
		{
			if (ImGui::BeginMenu("Current Camera"))
			{
				ImGui::Text("Name:");
				ImGui::Text(port.camera->object->name);
				ImGui::Separator();
				ImGui::NewLine();
				if (ImGui::MenuItem("Switch view type"))
				{
					App->renderer3D->FindViewPort(port.ID)->camera->SwitchViewType();
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();
			std::multimap<Component::Type, Component*>::iterator comp = App->GO->components.find(Component::Type::C_camera);
			for (; comp != App->GO->components.end() && comp->first == Component::Type::C_camera; comp++)
			{
				Camera* cam = (Camera*)&*comp->second;
				if (ImGui::MenuItem(cam->object->name))
				{
					App->renderer3D->FindViewPort(port.ID)->camera = cam;
					int a = 0;
				}
			}
			ImGui::EndMenu();
		}
		sprintf(tmp, "Switch View Type:##%i", port.ID);

		ImGui::EndMenuBar();
	}
	ImGui::End();
}

void ModuleEditor::SelectByViewPort()
{
	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		viewPort* port = nullptr;
		float2 portPos = App->renderer3D->ScreenToViewPort(float2(App->input->GetMouseX(), App->input->GetMouseY()), &port);
		//Checking the click was made on a port
		if (port != nullptr)
		{
			//Normalizing the mouse position in port to [-1,1]
			portPos.x = portPos.x / (port->size.x / 2) - 1;
			portPos.y = portPos.y / (port->size.y / 2) - 1;
			//Generating the LineSegment we'll check for collisions
			selectRay = port->camera->GetFrustum()->UnProjectLineSegment(portPos.x, -portPos.y);

			GameObject* out_go = NULL;

			if (App->GO->RayCast(selectRay, &out_go, &out_pos, &out_normal))
			{
				SelectGameObject(out_go);
			}
			else
			{
				SelectGameObject(nullptr);
			}

		}
	}
}
