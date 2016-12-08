#include "Globals.h"
#include "Application.h"

#include "ModuleGOmanager.h"

#include "ModuleInput.h"
#include "ModuleImporter.h"
#include "ModuleCamera3D.h"
#include "ModuleRenderer3D.h"
#include "ModuleFileSystem.h"
#include "imGUI\imgui.h"

#include "Mesh_RenderInfo.h"
#include "ViewPort.h"


#include "AllComponents.h"

#include <unordered_set>


//------------------------- MODULE --------------------------------------------------------------------------------

ModuleGoManager::ModuleGoManager(Application* app, bool start_enabled) : Module(app, start_enabled), quadTree(float3(WORLD_WIDTH /-2,WORLD_HEIGHT/-2,WORLD_DEPTH/-2), float3(WORLD_WIDTH / 2, WORLD_HEIGHT / 2, WORLD_DEPTH / 2))
{
	moduleName = "ModuleGeometry";
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
update_status ModuleGoManager::PreUpdate()
{
	update_status ret = UPDATE_CONTINUE;

	TIMER_START("Components PreUpdate");
	std::multimap<Component::Type, Component*>::iterator comp = components.begin();
	for (; comp != components.end(); comp++)
	{
		std::multimap<Component::Type, Component*>::iterator nextIt = comp;
		nextIt--;
		if (comp->second->TryDeleteNow())
		{
			comp = nextIt;
		}
		else if (comp->second->object->IsActive())
		{
			comp->second->PreUpdate();
		}
	}
	TIMER_READ_MS("Components PreUpdate");
	return ret;
}

update_status ModuleGoManager::Update()
{
	if (App->input->file_was_dropped)
	{
		char droppedFile[1024];
		strcpy(droppedFile, App->input->dropped_file);
		std::string file = App->importer->NormalizePath(droppedFile);
		LoadGO(file.data());
	}

	TIMER_START("Components Update");
	std::multimap<Component::Type, Component*>::iterator comp = components.begin();
	for (; comp != components.end(); comp++)
	{
		if (comp->second->object->IsActive())
		{
			comp->second->Update();
		}
	}
	TIMER_READ_MS("Components Update");
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

update_status ModuleGoManager::PostUpdate()
{
	TIMER_START("Components PostUpdate");
	std::multimap<Component::Type, Component*>::iterator comp = components.begin();
	for (; comp != components.end(); comp++)
	{
		if (comp->second->object->IsActive())
		{
			comp->second->PostUpdate();
		}
	}

	TIMER_READ_MS("Components PostUpdate");

	TIMER_RESET_STORED("Cam culling longest");
	TIMER_RESET_STORED("GO render longest");

	bool worked = false;

	if (wantToSaveScene && worked == false)
	{
		worked = true;
		TIMER_START_PERF("Saving Scene");
		SaveSceneNow();
		wantToSaveScene = false;
		TIMER_READ_MS("Saving Scene");
	}

	if (wantToClearScene && worked == false)
	{
		worked = true;
		ClearSceneNow();
		wantToClearScene = false;
	}

	DeleteGOs();

	if (wantToLoadScene&& worked == false)
	{
		worked = true;
		TIMER_START_PERF("Loading Scene");
		LoadSceneNow();
		wantToLoadScene = false;
		TIMER_READ_MS("Loading Scene");
	}


	return UPDATE_CONTINUE;
}

void ModuleGoManager::Render(const viewPort& port)
{
	App->GO->RenderGOs(port);
	if (drawQuadTree)
	{
		TIMER_START("QuadTree drawTime");
		quadTree.Draw();
		TIMER_READ_MS("QuadTree drawTime");
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


//Creating/deleting GOs

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

GameObject * ModuleGoManager::DuplicateGO(GameObject * toCopy)
{
	GameObject* ret = new GameObject();

	return nullptr;
}

std::vector<GameObject*> ModuleGoManager::LoadGO(const char* fileName)
{
	GameObject* sceneRoot = App->importer->LoadVgo(fileName, "RootNode");
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


//Scene management

void ModuleGoManager::ClearSceneNow()
{
	if (root->childs.empty() == false)
	{
		std::vector<GameObject*>::iterator it = root->childs.begin();
		for (; it != root->childs.end(); it++)
		{
			if ((*it)->HiddenFromOutliner() == false)
			{
				toDelete.push(*it);
			}
		}
			LOG("Scene cleared");
	}
}

void ModuleGoManager::SaveSceneNow()
{
	// xml object were we will store all data
	pugi::xml_document data;
	pugi::xml_node root_node;
	pugi::xml_node Components_node;

	root_node = data.append_child("Scene");

	root_node.append_attribute("SceneName") = sceneName.data();

	//Saving GameObjects
	root->Save(root_node.append_child("GameObjects"));

	Components_node = root_node.append_child("Components");

	//Saving components
	std::multimap<Component::Type, Component*>::iterator comp = components.begin();
	for (; comp != components.end(); comp++)
	{
		if (comp->second->object->HiddenFromOutliner() == false)
		{
			comp->second->Save(Components_node.append_child("Component"));
		}
	}

	char path[524];
	sprintf(path, "Assets/Scenes/%s%s", sceneName.data(), SCENE_FORMAT);

	std::stringstream stream;
	data.save(stream);
	// we are done, so write data to disk
	App->fs->Save(path, stream.str().c_str(), stream.str().length());
	LOG("Scene saved: %s", path);

	data.reset();
}

void ModuleGoManager::LoadSceneNow()
{
	std::map<uint64_t, GameObject*> UIDlib;

	char scenePath[526];
	sprintf(scenePath, "Assets/Scenes/%s%s", sceneName.data(), SCENE_FORMAT);


	char* buffer;
	uint size = App->fs->Load(scenePath, &buffer);

	if (size > 0)
	{
		pugi::xml_document data;
		pugi::xml_node root;

		pugi::xml_parse_result result = data.load_buffer(buffer, size);
		RELEASE(buffer);

		if (result != NULL)
		{
			root = data.child("Scene");
			if (root)
			{
				for (pugi::xml_node GOs = root.child("GameObjects").first_child(); GOs != nullptr; GOs = GOs.next_sibling())
				{
					uint64_t parentUID = GOs.attribute("parent").as_ullong();
					std::string go_name = GOs.attribute("name").as_string();
					
					uint64_t UID = GOs.attribute("UID").as_ullong();

					GameObject* toAdd = new GameObject();
					toAdd->SetName(go_name.data());

					bool isStatic = GOs.attribute("Static").as_bool();
					toAdd->SetStatic(isStatic);

					bool isActive = GOs.attribute("Active").as_bool();
					toAdd->SetActive(isActive);

					std::map<uint64_t, GameObject*>::iterator parent = UIDlib.find(parentUID);
					if (parent != UIDlib.end())
					{
						toAdd->parent = parent->second;
						parent->second->childs.push_back(toAdd);
					}
					UIDlib.insert(std::pair<uint64_t, GameObject*>(UID, toAdd));
					if (UID != 0)
					{
						dynamicGO.push_back(toAdd);
					}
				}

				for (pugi::xml_node comp = root.child("Components").first_child(); comp != nullptr; comp = comp.next_sibling())
				{
					pugi::xml_node general = comp.child("General");
					std::string name = general.attribute("name").as_string();
					uint64_t UID = general.attribute("UID").as_ullong();
					Component::Type type = (Component::Type)general.attribute("type").as_int();
					int id = general.attribute("id").as_int();

					uint64_t GO = general.attribute("GO").as_ullong();

					bool enabled = general.attribute("enabled").as_bool();
					if (GO != 0)
					{
						std::map<uint64_t, GameObject*>::iterator go = UIDlib.find(GO);
						if (go != UIDlib.end())
						{
							Component* c = go->second->AddComponent(type, "", true);
							if (c != nullptr)
							{
								c->LoadSpecifics(comp.child("Specific"));
							}
						}
					}
				}

				GameObject* sceneRoot = UIDlib.find(0)->second;
				for (std::vector<GameObject*>::iterator it = sceneRoot->childs.begin(); it != sceneRoot->childs.end(); it++)
				{
					AddGOtoRoot((*it));
				}
				//Deleting a Gameobject will also delete and clear all his childs. In this special case we don't want that
				sceneRoot->childs.clear();
				RELEASE(sceneRoot);

				LOG("Scene loaded: %s", sceneName.data());
			}
		}
	}

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



bool ModuleGoManager::RayCast(const LineSegment & ray, GameObject** OUT_gameobject, float3 * OUT_position, float3* OUT_normal, bool onlyMeshes)
{
	TIMER_RESET_STORED("Raycast");
	TIMER_START("Raycast");
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
		if ((*GO)->obb.Intersects(ray, distanceNear, distanceFar) == true)
		{
			candidates.insert(std::pair<float, GameObject*>(MIN(distanceNear, distanceFar), (*GO)));
		}
	}

	//Checking all the possible collisions in order
	for (std::map<float, GameObject*>::iterator check = candidates.begin(); check != candidates.end() && collided == false && candidates.empty() == false; check++)
	{
		float collisionDistance = floatMax;
		//One object may have more than a single mesh, so we'll check them one by one
		if (check->second->HasComponent(Component::Type::C_mesh))
		{
			std::vector<mesh*> meshes = check->second->GetComponent<mesh>();
			for (std::vector<mesh*>::iterator m = meshes.begin(); m != meshes.end(); m++)
			{
				LineSegment transformedRay = ray;
				transformedRay.Transform(check->second->GetTransform()->GetGlobalTransform().InverseTransposed());
				//Generating the triangles the mes has, and checking them one by one
				const float3* vertices = (*m)->GetVertices();
				const uint* index = (*m)->GetIndices();
				for (int n = 0; n < (*m)->GetNumIndices(); n += 3)
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
			}
		}
		else if(onlyMeshes == false)
		{
			collided = true;
			out_go = check->second;
			out_normal = float3(0, 1, 0);
			out_pos = check->second->GetTransform()->GetGlobalPos();
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
	TIMER_READ_MS("Raycast");
	return collided;
}


Mesh_RenderInfo ModuleGoManager::GetMeshData(mesh * getFrom)
{
	Mesh_RenderInfo ret = getFrom->GetMeshInfo();

	ret.transform = getFrom->object->GetTransform()->GetGlobalTransform();

	if (getFrom->object->HasComponent(Component::Type::C_material))
	{
		Material* mat = getFrom->object->GetComponent<Material>().front();
		if (mat->toDelete == false)
		{
			ret.meshColor = mat->GetColor();
			ret.textureBuffer = mat->GetTexture(getFrom->texMaterialIndex);
			ret.alphaType = mat->GetAlphaType();
			ret.alphaTest = mat->GetAlphaTest();
			ret.blendType = mat->GetBlendType();
		}
	}
	else
	{
		ret.meshColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	return ret;
}

void ModuleGoManager::RenderGOs(const viewPort & port, const std::vector<GameObject*>& exclusiveGOs)
{
	std::unordered_set<GameObject*> toRender;

	//This vector will generally be empty. It is only used when we send certain GOs we want to render exclusively
	if (exclusiveGOs.empty() == true)
	{
		//Call the Draw function of all the components, so they do what they need to
		std::multimap<Component::Type, Component*>::iterator comp = components.begin();
		for (; comp != components.end(); comp++)
		{
			if (comp->second->object->IsActive())
			{
				comp->second->Draw();
				if (comp->second->object->HasComponent(Component::Type::C_Billboard))
				{
					Transform* camTransform = port.camera->object->GetTransform();
					comp->second->object->GetComponent<Billboard>().front()->UpdateNow(camTransform->GetGlobalPos(), camTransform->Up());
				}
			}
		}
		TIMER_START("Cam culling longest");
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
		TIMER_READ_MS_MAX("Cam culling longest");
	}
	else
	{
		App->renderer3D->SetViewPort(*App->renderer3D->FindViewPort(port.ID));
		for (std::vector<GameObject*>::const_iterator toInsert = exclusiveGOs.begin(); toInsert != exclusiveGOs.end(); toInsert++)
		{
			toRender.insert(*toInsert);
		}
	}

	TIMER_START("GO render longest");
	//And now, we render them
	TIMER_RESET_STORED("Mesh slowest");
	for (std::unordered_set<GameObject*>::iterator it = toRender.begin(); it != toRender.end(); it++)
	{
		if ((*it)->HasComponent(Component::Type::C_mesh))
		{
			std::vector<mesh*> meshes = (*it)->GetComponent<mesh>();
			if (meshes.empty() == false)
			{
				for (std::vector<mesh*>::iterator mesh = meshes.begin(); mesh != meshes.end(); mesh++)
				{
					if ((*mesh)->IsEnabled() && (*mesh)->toDelete == false)
					{
						TIMER_START("Mesh slowest");
						Mesh_RenderInfo info = GetMeshData(*mesh);
						if (port.useOnlyWires)
						{
							info.filled = false;
							info.wired = true;
						}
						App->renderer3D->DrawMesh(info);
						TIMER_READ_MS_MAX("Mesh slowest");
					}
				}
			}
		}
	}
	TIMER_READ_MS_MAX("GO render longest");
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
