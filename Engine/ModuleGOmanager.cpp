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
		if (ImGui::BeginPopupModal("##SetStaticChilds", &StaticChildsPopUpIsOpen))
		{
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
			ImGui::EndPopup();
		}
		if (StaticChildsPopUpIsOpen == false)
		{
			ImGui::OpenPopup("##SetStaticChilds");
			StaticChildsPopUpIsOpen = true;
		}
	}
	else
	{
		StaticChildsPopUpIsOpen = false;
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

void ModuleGoManager::Render(const viewPort& port)
{
	App->GO->RenderGOs(port);
	if (drawQuadTree)
	{
		quadTree.Draw();
	}
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

bool ModuleGoManager::RayCast(const LineSegment & ray, GameObject** OUT_gameobject, float3 * OUT_position, float3* OUT_normal)
{
	bool collided = false;
	GameObject* out_go = NULL;
	float3 out_pos = float3::zero;
	float3 out_normal = float3::zero;

	//Obtaining all the AABB collisions, and sorting them by distance of the AABB
	std::vector<GameObject*> colls = App->GO->FilterCollisions(ray);
	std::map<float, GameObject*> candidates;
	for (std::vector<GameObject*>::iterator GO = colls.begin(); GO != colls.end() && colls.empty() == false; GO++)
	{
		float distanceNear;
		float distanceFar;
		//The distance is normalized between [0,1] and is the relative position in the Segment the AABB collides
		if ((*GO)->aabb.Intersects(ray, distanceNear, distanceFar) == true)
		{
			if ((*GO)->obb.Intersects(ray, distanceNear, distanceFar) == true)
			{
				candidates.insert(std::pair<float, GameObject*>(distanceNear, (*GO)));
			}
		}
	}

	//Checking all the possible collisions in order
	for (std::map<float, GameObject*>::iterator check = candidates.begin(); check != candidates.end() && collided == false && candidates.empty() == false; check++)
	{
		float collisionDistance = floatMax;
		//One object may have more than a single mesh, so we'll check them one by one
		std::vector<mesh*> meshes = check->second->GetComponent<mesh>();
		for (std::vector<mesh*>::iterator m = meshes.begin(); m != meshes.end(); m++)
		{
			LineSegment transformedRay = ray;
			transformedRay.Transform(check->second->GetTransform()->GetGlobalTransform().InverseTransposed());
			//Generating the triangles the mes has, and checking them one by one
			float3* vertices = (*m)->GetVertices();
			uint* index = (*m)->GetIndices();
			for (int n = 0; n < (*m)->num_indices; n+=3)
			{
				Triangle tri(vertices[index[n]], vertices[index[n + 1]], vertices[index[n + 2]]);
				float3 intersectionPoint;
				float distance;
				//If the triangle we collided with is further away than a previous collision, we'll ignore it
				if (tri.Intersects(transformedRay, &distance, &intersectionPoint) == true)
				{
					if (distance < collisionDistance)
					{
						collided = true;
						collisionDistance = distance;
						out_go = check->second;
						out_pos = intersectionPoint;
						out_normal = tri.NormalCCW();
						LineSegment tmp(out_pos, out_pos + out_normal);
						tmp.Transform(check->second->GetTransform()->GetGlobalTransform().Transposed());
						out_pos = tmp.a;
						out_normal = tmp.b - tmp.a;
					}
				}
			}
			RELEASE_ARRAY(vertices);
			RELEASE_ARRAY(index);
		}
	}
	*OUT_gameobject = out_go;
	if (OUT_normal != NULL)
	{
		*OUT_normal = out_normal.Normalized();
	}
	if (OUT_position != NULL)
	{
		*OUT_position = out_pos;
	}

	return collided;
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

void ModuleGoManager::RenderGOs(const viewPort & port)
{
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
