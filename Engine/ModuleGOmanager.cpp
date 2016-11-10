#include "Globals.h"
#include "Application.h"

#include "ModuleGOmanager.h"

#include "ModuleInput.h"
#include "ModuleImporter.h"
#include "ModuleCamera3D.h"
#include "ModuleRenderer3D.h"
#include "imGUI\imgui.h"

#include "Mesh_RenderInfo.h"
#include "ViewPort.h"


#include "AllComponents.h"

#include <unordered_set>


//------------------------- MODULE --------------------------------------------------------------------------------

ModuleGoManager::ModuleGoManager(Application* app, bool start_enabled) : Module(app, start_enabled), quadTree(float3(WORLD_WIDTH /-2,WORLD_HEIGHT/-2,WORLD_DEPTH/-2), float3(WORLD_WIDTH / 2, WORLD_HEIGHT / 2, WORLD_DEPTH / 2))
{
	name.create("ModuleGeometry");
}

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

	std::multimap<Component::Type, Component*>::iterator comp = components.begin();
	for (; comp != components.end(); comp++)
	{
		if (comp->second->object->IsActive())
		{
			comp->second->PreUpdate();
		}
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

	std::multimap<Component::Type, Component*>::iterator comp = components.begin();
	for (; comp != components.end(); comp++)
	{
		if (comp->second->object->IsActive())
		{
			comp->second->Update();
		}
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

	return UPDATE_CONTINUE;
}

update_status ModuleGoManager::PostUpdate(float dt)
{
	std::multimap<Component::Type, Component*>::iterator comp = components.begin();
	for (; comp != components.end(); comp++)
	{
		if (comp->second->object->IsActive())
		{
			comp->second->PostUpdate();
		}
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
	
	if (name != NULL && name != "")
	{
		empty->SetName(name);
	}

	AddGOtoRoot(empty);

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


Mesh_RenderInfo ModuleGoManager::GetMeshData(mesh * getFrom)
{
	Mesh_RenderInfo ret = getFrom->GetMeshInfo();

	ret.transform = getFrom->object->GetTransform()->GetGlobalTransform();

	if (getFrom->object->HasComponent(Component::Type::C_material))
	{
		Material* mat = getFrom->object->GetComponent<Material>().front();
		ret.meshColor = mat->GetColor();
		ret.textureBuffer = mat->GetTexture(getFrom->texMaterialIndex);
	}
	return ret;
}

void ModuleGoManager::RenderGOs(viewPort & port)
{
	App->renderer3D->SetViewPort(port);

	if (drawQuadTree)
	{
		quadTree.Draw();
	}

	//Call the Draw function of all the components, so they do what they need to
	std::multimap<Component::Type, Component*>::iterator comp = components.begin();
	for (; comp != components.end(); comp++)
	{
		if (comp->second->object->IsActive())
		{
			comp->second->Draw();
		}
	}

	std::unordered_set<GameObject*> toRender;
	bool aCamHadCulling = false;
	//Finding all the cameras that have culling on, and collecting all the GOs we need to render
	std::multimap<Component::Type, Component*>::iterator it = components.find(Component::Type::C_camera);
	for (; it != components.end() && it->first == Component::Type::C_camera; it++)
	{
		if (((Camera*)(it->second))->HasCulling())
		{
			aCamHadCulling = true;
			std::vector<GameObject*> GOs;
			//If a camera has ortographiv view, we'll need to test culling against an AABB instead of against it frustum
			if (((Camera*)(it->second))->GetFrustum()->type == FrustumType::PerspectiveFrustum)
			{
				GOs = FilterCollisions(*((Camera*)(it->second))->GetFrustum());
			}
			else
			{
				GOs = FilterCollisions(((Camera*)(it->second))->GetFrustum()->MinimalEnclosingAABB());
			}
			for (std::vector<GameObject*>::iterator toInsert = GOs.begin(); toInsert != GOs.end(); toInsert++)
			{
				toRender.insert(*toInsert);
			}
		}
	}

	//If no cameras had culling active, we'll cull from the Current Active camera
	if (aCamHadCulling == false)
	{
		std::vector<GameObject*> GOs;
		if (port.camera->GetFrustum()->type == FrustumType::PerspectiveFrustum)
		{
			GOs = FilterCollisions(*port.camera->GetFrustum());
		}
		else
		{
			GOs = FilterCollisions(port.camera->GetFrustum()->MinimalEnclosingAABB());
		}
		for (std::vector<GameObject*>::iterator toInsert = GOs.begin(); toInsert != GOs.end(); toInsert++)
		{
			toRender.insert(*toInsert);
		}
	}

	//And now, we render them
	for (std::unordered_set<GameObject*>::iterator it = toRender.begin(); it != toRender.end(); it++)
	{
		std::vector<mesh*> meshes = (*it)->GetComponent<mesh>();
		if (meshes.empty() == false)
		{
			for (std::vector<mesh*>::iterator mesh = meshes.begin(); mesh != meshes.end(); mesh++)
			{
				App->renderer3D->DrawMesh(GetMeshData(*mesh));
			}
		}
	}
}

void ModuleGoManager::AddGOtoRoot(GameObject * GO)
{
	GO->parent = root;
	root->childs.push_back(GO);
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
		delete toDelete.top();
		toDelete.pop();
	}
}
