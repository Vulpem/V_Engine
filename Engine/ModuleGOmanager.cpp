#include "Globals.h"
#include "Application.h"

#include "ModuleGOmanager.h"

#include "ModuleInput.h"
#include "ModuleImporter.h"
#include "imGUI\imgui.h"

#include "AllComponents.h"



//------------------------- MODULE --------------------------------------------------------------------------------

ModuleGoManager::ModuleGoManager(Application* app, bool start_enabled) : Module(app, start_enabled), quadTree(float3(-300,-10,-300), float3(300, 10, 300))
{
	name.create("ModuleGeometry");
}

// DestructorF
ModuleGoManager::~ModuleGoManager()
{

}

// Called before render is available
bool ModuleGoManager::Init()
{
	bool ret = true;

	CreateRootGameObject();
		
	return ret;
}

bool ModuleGoManager::Start()
{
	return true;
}

// Called every draw update
update_status ModuleGoManager::PreUpdate(float dt)
{
	update_status ret = UPDATE_CONTINUE;

	std::vector<GameObject*>::iterator it = root->childs.begin();
	while (it != root->childs.end())
	{
		(*it)->PreUpdate();
		it++;
	}

	return ret;
}

update_status ModuleGoManager::Update(float dt)
{
	if (App->input->file_was_dropped)
	{
		char droppedFile[1024];
		strcpy(droppedFile, App->input->dropped_file);
		std::string onlyName = App->importer->FileName(droppedFile);
		LoadGO(onlyName.data());
	}

	std::vector<GameObject*>::iterator it = root->childs.begin();
	while (it != root->childs.end())
	{
		(*it)->Update();
		it++;
	}

	if (setting != nullptr)
	{
		ImGui::SetNextWindowSize(ImVec2(250, 75));

		ImGui::Begin("##SetStaticChilds");
		if (settingStatic)
		{
			ImGui::Text("Set childs to Static too?");
		}
		else
		{
			ImGui::Text("Set childs to non Static too?");
		}
		ImGui::NewLine();
		ImGui::SameLine(30);
		if (ImGui::Button("Yes##yesSetStatic"))
		{
			SetChildsStatic(settingStatic, setting);
			setting = nullptr;
		}
		ImGui::SameLine(150);
		if (ImGui::Button("No##NoSetStatic"))
		{
			SetStatic(settingStatic, setting);
			setting = nullptr;
		}
		ImGui::End();
	}

	if (drawQuadTree)
	{
		quadTree.Draw();
	}

	return UPDATE_CONTINUE;
}

update_status ModuleGoManager::PostUpdate(float dt)
{
	std::vector<GameObject*>::iterator it = root->childs.begin();
	while (it != root->childs.end())
	{
		(*it)->PostUpdate();
		it++;
	}

	DeleteGOs();
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleGoManager::CleanUp()
{
	if (root)
	{
		delete root;
	}

	return true;
}

GameObject * ModuleGoManager::CreateEmpty(const char* name)
{
	GameObject* empty = new GameObject();

	empty->AddComponent(Component::Type::C_transform);

	AddGOtoRoot(empty);
	
	if (name != NULL && name != "")
	{
		empty->SetName(name);
	}

	return empty;
}

GameObject* ModuleGoManager::CreateCamera(const char* name)
{
	GameObject* camera = CreateEmpty(name);
	camera->AddComponent(Component::Type::C_camera);
	return camera;
}

std::vector<GameObject*> ModuleGoManager::LoadGO(const char* fileName)
{
	GameObject* sceneRoot = App->importer->LoadVgo(fileName);
	std::vector<GameObject*> ret;
	if (sceneRoot && sceneRoot->childs.empty() == false)
	{
		for (std::vector<GameObject*>::iterator childs = sceneRoot->childs.begin(); childs != sceneRoot->childs.end(); childs++)
		{
			AddGOtoRoot(*childs);
			ret.push_back((*childs));
		}
		//Deleting a Gameobject will also delete and clear all his childs. In this special case we don't want that
		sceneRoot->childs.clear();
		delete sceneRoot;
		LOG("Loaded %s", fileName);
	}
	else
	{
		LOG("Failed to load %s", fileName);
	}
	return ret;
}

bool ModuleGoManager::DeleteGameObject(GameObject* toErase)
{
	if (toErase)
	{
		toDelete.push(toErase);
		return true;
	}
	return false;
	
}

void ModuleGoManager::SetStatic(bool Static, GameObject * GO)
{
	if (Static != GO->IsStatic())
	{
		GO->SetStatic(Static);
		if (Static)
		{
			if (GO->parent != nullptr)
			{
				SetStatic(true, GO->parent);
			}
			App->GO->quadTree.Add(GO);
			for (std::vector<GameObject*>::iterator it = App->GO->dynamicGO.begin(); it != App->GO->dynamicGO.end(); it++)
			{
				if ((*it) == GO)
				{
					dynamicGO.erase(it);
					break;
				}
			}
		}
		else
		{
			if (GO->childs.empty() == false)
			{
				for (std::vector<GameObject*>::iterator it = GO->childs.begin(); it != GO->childs.end(); it++)
				{
					SetStatic(false, (*it));
				}
			}
			quadTree.Remove(GO);
			dynamicGO.push_back(GO);
		}
	}
}

void ModuleGoManager::SetChildsStatic(bool Static, GameObject * GO)
{
	SetStatic(Static, GO);
	if (Static == true)
	{
		if (GO->childs.empty() == false)
		{
			for (std::vector<GameObject*>::iterator it = GO->childs.begin(); it != GO->childs.end(); it++)
			{
				SetChildsStatic(Static,(*it));
			}
		}
	}
}

std::vector<GameObject*> ModuleGoManager::FilterCollisions(LineSegment col)
{
	std::vector<GameObject*> ret = quadTree.FilterCollisions(col);

	for (std::vector<GameObject*>::iterator it = dynamicGO.begin(); it != dynamicGO.end(); it++)
	{
		if ((*it)->aabb.Intersects(col) == true)
		{
			ret.push_back(*it);
		}
	}

	return ret;
}

std::vector<GameObject*> ModuleGoManager::FilterCollisions(AABB col)
{
	std::vector<GameObject*> ret = quadTree.FilterCollisions(col);

	for (std::vector<GameObject*>::iterator it = dynamicGO.begin(); it != dynamicGO.end(); it++)
	{
		if ((*it)->aabb.Intersects(col) == true)
		{
			ret.push_back(*it);
		}
	}

	return ret;
}

void ModuleGoManager::AddGOtoRoot(GameObject * GO)
{
	GO->parent = root;
	root->childs.push_back(GO);
	SetUpGO(GO);
}

void ModuleGoManager::SetUpGO(GameObject * GO)
{
	dynamicGO.push_back(GO);

	for (std::vector<GameObject*>::iterator it = GO->childs.begin(); it != GO->childs.end(); it++)
	{
		SetUpGO(*it);
	}
}

void ModuleGoManager::CreateRootGameObject()
{
	if (root == nullptr)
	{
		GameObject* ret = new GameObject();

		LOG("Creating root node for scene");
		//Setting Name
		ret->SetName("Root");

		//Setting parent
		ret->parent = nullptr;

		//Setting transform
		math::Quat rot = math::Quat::identity;

		root = ret;
	}
	else
	{
		LOG("Be careful! You almost created a second root node!");
	}
}

void ModuleGoManager::DeleteGOs()
{
	while (toDelete.empty() == false)
	{
		LOG("Erasing GO %s", toDelete.top()->GetName());
		SetStatic(false, toDelete.top());
		if (dynamicGO.empty() == false)
		{
			for (std::vector<GameObject*>::iterator it = dynamicGO.begin(); it != dynamicGO.end(); it++)
			{
				if ((*it) == toDelete.top())
				{
					dynamicGO.erase(it);
					break;
				}
			}
		}		
		delete toDelete.top();
		toDelete.pop();
	}
}
