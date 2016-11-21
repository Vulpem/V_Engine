#include "GameObject.h"

#include "AllComponents.h"

#include "imGUI\imgui.h"

#include "OpenGL.h"
#include <map>

#include "Application.h"
#include "ModuleCamera3D.h"
#include "ModuleRenderer3D.h"
#include "ModuleGOmanager.h"


GameObject::GameObject()
{
	uid = GenerateUUID();
	for (int n = 0; n < nComponentTypes; n++)
	{
		HasComponents[n] = false;
	}
	aabb.SetNegativeInfinity();
	originalAABB.SetNegativeInfinity();
	strcpy(name, "Unnamed");
	App->GO->dynamicGO.push_back(this);
}

GameObject::~GameObject()
{
	if (IsStatic() == false)
	{
		if (App->GO->dynamicGO.empty() == false)
		{
			for (std::vector<GameObject*>::iterator it = App->GO->dynamicGO.begin(); it != App->GO->dynamicGO.end(); it++)
			{
				if ((*it) == this)
				{
					App->GO->dynamicGO.erase(it);
					break;
				}
			}
		}
	}
	else
	{
		App->GO->quadTree.Remove(this);
	}

	if (parent != nullptr)
	{
		std::vector<GameObject*>::iterator it = parent->childs.begin();
		while ((*it) != this)
		{
			it++;
		}
		parent->childs.erase(it);
	}

	if (childs.empty() == false)
	{
		std::vector<GameObject*>::iterator iterator = childs.begin();
		while (childs.size() > 0 && iterator != childs.end())
		{
			delete (*iterator);
			//Erasing a Node will already remove it from the child list in its destructor, so we don't have to empty the list here, it will be done automatically
			if (childs.size() > 0)
			{
				iterator = childs.begin();
			}
		}
	}

	std::vector<Component*>::reverse_iterator comp = components.rbegin();
	while (comp != components.rend())
	{
		std::multimap<Component::Type, Component*>::iterator it = App->GO->components.find((*comp)->GetType());
		for (; it->first == (*comp)->GetType(); it++)
		{		
			if (it->second == (*comp))
			{
				App->GO->components.erase(it);
				break;
			}
		}

		delete *comp;
		comp++;
	}
	components.clear();
}

void GameObject::DrawOnEditor()
{
	if (ImGui::BeginPopup("Add Component"))
	{
		if (ImGui::Button("Camera##add"))
		{
			AddComponent(Component::Type::C_camera);
		}
		ImGui::EndPopup();
	}

	bool isActive = IsActive();
	ImGui::Checkbox("", &isActive);
	if (isActive != IsActive())
	{
		SetActive(isActive);
	}
	ImGui::SameLine();
	ImGui::Text("Name:");
	ImGui::SameLine();
	ImGui::InputText("##Name", name, NAME_MAX_LEN);

	if (ImGui::Button("Add:"))
	{
		ImGui::OpenPopup("Add Component");
	}
	ImGui::SameLine();
	ImGui::Text("Static: ");
	ImGui::SameLine();
	bool isStatic = Static;
	ImGui::Checkbox("##isObjectStatic", &isStatic);
	if (isStatic != Static && App->GO->setting == nullptr)
	{
		if (childs.empty() == true)
		{
			App->GO->SetStatic(isStatic, this);
		}
		else
		{
			App->GO->setting = this;
			App->GO->settingStatic = isStatic;
		}
	}

	for (std::vector<Component*>::iterator it = components.begin(); it != components.end(); it++)
	{
		(*it)->DrawOnEditor();
	}
}

void GameObject::DrawLocator()
{
	float4 color = float4(0.1f, 0.58f, 0.2f, 1.0f);
	if (selected)
	{
		if (parent->selected)
		{
			color = float4(0, 0.5f, 0.5f, 1);
		}
		else {
			color = float4(0, 0.8f, 0.8f, 1);
		}
	}

	App->renderer3D->DrawLocator(GetTransform()->GetGlobalTransform(), color);

	if (childs.empty() == false)
	{
		for (std::vector<GameObject*>::iterator it = childs.begin(); it != childs.end(); it++)
		{
			if ((*it)->HasComponent(Component::Type::C_transform) && !(*it)->HasComponent(Component::Type::C_mesh))
			{
				math::float3 childPos((*it)->GetTransform()->GetGlobalPos());
				App->renderer3D->DrawLine(GetTransform()->GetGlobalPos(), childPos, color);
			}
		}
	}

}

void GameObject::DrawAABB()
{
	if (aabb.IsFinite())
	{
		math::float3 corners[8];
		aabb.GetCornerPoints(corners);
		App->renderer3D->DrawBox(corners);
	}
}

void GameObject::DrawOBB()
{
	if (obb.IsFinite())
	{
		math::float3 corners[8];
		obb.GetCornerPoints(corners);
		App->renderer3D->DrawBox(corners, float4(0.2f, 0.45f, 0.27f, 1.0f));
	}
}

void GameObject::Select(bool _renderNormals)
{
	selected = true;
	renderNormals = _renderNormals;

	GetTransform()->UpdateEditorValues();

	std::vector<GameObject*>::iterator childIt = childs.begin();
	while (childIt != childs.end())
	{
		(*childIt)->Select();
		childIt++;
	}
}

void GameObject::Unselect()
{
	selected = false;
	renderNormals = false;
	std::vector<GameObject*>::iterator childIt = childs.begin();
	while (childIt != childs.end())
	{
		(*childIt)->Unselect();
		childIt++;
	}
}

void GameObject::SetOriginalAABB(float3 minPoint, float3 maxPoint)
{
	originalAABB.minPoint = minPoint;
	originalAABB.maxPoint = maxPoint;
	if (originalAABB.IsFinite() == false)
	{
		originalAABB.minPoint = float3{ -0.25f,-0.25f,-0.25f };
		originalAABB.maxPoint = float3{ 0.25f,0.25f,0.25f };
	}

	UpdateAABB();
}

void GameObject::UpdateAABB()
{
	aabb.SetNegativeInfinity();
	obb.SetNegativeInfinity();
	if (originalAABB.IsFinite())
	{
		obb = originalAABB;
		obb.Transform(GetTransform()->GetGlobalTransform().Transposed());
		aabb.Enclose(obb);
	}
}

void GameObject::UpdateTransformMatrix()
{
	if (HasComponent(Component::Type::C_transform))
	{	
		GetTransform()->UpdateGlobalTransform();
	}

	UpdateAABB();

	//Updating cameras position
	if (HasComponent(Component::Type::C_camera))
	{
		std::vector<Camera*> cams = GetComponent<Camera>();
		std::vector<Camera*>::iterator it = cams.begin();
		while (it != cams.end())
		{
			(*it)->UpdateCamMatrix();
			it++;
		}
	}

	if (childs.empty() == false)
	{
		std::vector<GameObject*>::iterator child = childs.begin();
		while (child != childs.end())
		{
			(*child)->UpdateTransformMatrix();
			child++;
		}
	}

}

void GameObject::SetActive(bool state, bool justPublic)
{
	if (state == publicActive)
	{
		return;
	}
	publicActive = state;

		std::vector<GameObject*>::iterator childIt = childs.begin();
		while (childIt != childs.end())
		{
			(*childIt)->SetActive(state, true);
			childIt++;
		}

	if (justPublic)
	{		
		return;
	}
	active = state;

	if(state == true && parent)
	{
		parent->SetActive(true);
	}
}

bool GameObject::IsActive()
{
	if (active == false)
	{
		return false;
	}
	return publicActive; 
}


void GameObject::SetName(const char * newName)
{
	strcpy(name, newName);
}

const char * GameObject::GetName()
{
	return name;
}

Component* GameObject::AddComponent(Component::Type type)
{
	Component* toAdd = nullptr;
	switch (type)
	{
	case Component::Type::C_transform:
	{
		if (HasComponent(Component::C_transform) == false)
		{
			toAdd = new Transform(this, components.size()); 
			transform = (Transform*)toAdd;
		}
		break;
	}
	case Component::Type::C_mesh:
	{
		toAdd = new mesh(this, components.size()); break;
	}
	case Component::Type::C_material:
	{
		if (HasComponent(Component::C_material) == false)
		{
			toAdd = new Material(this, components.size());
		}
		break;
	}
	case Component::Type::C_camera:
	{
		if (HasComponent(Component::Type::C_transform))
		{
			toAdd = new Camera(this, components.size());
		}
		break;
	}
	}

	if (toAdd != nullptr)
	{
		HasComponents[type] = true;
		components.push_back(toAdd);
	}
	App->GO->components.insert(std::pair<Component::Type, Component*>(toAdd->GetType(), toAdd));

	return toAdd;
}

bool GameObject::HasComponent(Component::Type type)
{
	return HasComponents[type];
	/*if (components.size() > 0)
	{
		if (type == Component::Type::C_transform)
		{
			if (transform == nullptr)
			{
				return false;
			}
			return true;
		}
		std::vector<Component*>::iterator it = components.begin();
		while (it != components.end())
		{
			if ((*it)->GetType() == type)
			{
				return true;
			}
			it++;
		}
	}
	return false;*/
}

Transform * GameObject::GetTransform()
{
	return transform;
}

void GameObject::Save(pugi::xml_node node)
{
	pugi::xml_node GO = node.append_child("GO");
	GO.append_attribute("name") = name;
	GO.append_attribute("UID") = uid;
	GO.append_attribute("parent") = parent->GetUID();

	pugi::xml_node aabb_node = GO.append_child("Original_AABB");

	aabb_node.append_attribute("minX") = originalAABB.minPoint.x;
	aabb_node.append_attribute("minY") = originalAABB.minPoint.y;
	aabb_node.append_attribute("minZ") = originalAABB.minPoint.z;

	aabb_node.append_attribute("maxX") = originalAABB.maxPoint.x;
	aabb_node.append_attribute("maxY") = originalAABB.maxPoint.y;
	aabb_node.append_attribute("maxZ") = originalAABB.maxPoint.z;

	for (std::vector<GameObject*>::iterator it = childs.begin(); it != childs.end(); it++)
	{
		(*it)->Save(node);
	}
}
