#include "GameObject.h"

#include "AllComponents.h"

#include "imGUI\imgui.h"

#include "OpenGL.h"

#include "Application.h"
#include "ModuleCamera3D.h"
#include "ModuleRenderer3D.h"
#include "ModuleGOmanager.h"

//------------------------- NODE --------------------------------------------------------------------------------
GameObject::GameObject()
{
	aabb.SetNegativeInfinity();
	originalAABB.SetNegativeInfinity();
	strcpy(name, "Unnamed");
}


GameObject::~GameObject()
{

	SetStatic(false);

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

	std::vector<Component*>::iterator comp = components.begin();
	while (comp != components.end())
	{
		delete *comp;
		comp++;
	}

}

void GameObject::PreUpdate()
{
	if (active)
	{
		disabledByCulling = false;
		if (App->camera->cullingCameras.empty() == false)
		{
			if (aabb.IsFinite())
			{
				for (std::vector<Camera*>::iterator it = App->camera->cullingCameras.begin(); it != App->camera->cullingCameras.end(); it++)
				{
					if ((*it)->Collides(aabb) == FrustumCollision::outside)
					{
						disabledByCulling = true;
					}
				}
			}
			else
			{
				disabledByCulling = true;
			}
		}

		for (std::vector<Component*>::iterator it = components.begin(); it != components.end(); it++)
		{
			(*it)->PreUpdate();
		}
		std::vector<GameObject*>::iterator it = childs.begin();
		while (it != childs.end())
		{
			(*it)->PreUpdate();
			it++;
		}
	}
}

void GameObject::Update()
{
	if (active)
	{
		glPushMatrix();

		if (HasComponent(Component::Type::C_transform))
		{			
			glMultMatrixf(GetTransform()->GetGlobalTransform().ptr());
		}

		for (std::vector<Component*>::iterator it = components.begin(); it != components.end(); it++)
		{
			(*it)->Update();
		}

		if (HasComponent(Component::Type::C_mesh) == false)
		{
			DrawLocator();
		}
		glPopMatrix();

		//Drawing AABB independantly of object transform
		if (drawAABB && selected)
		{
			DrawAABB();
		}

		//Drawing Camera independantly of object transform
		std::vector<Camera*> cams = GetComponent<Camera>();
		if (cams.empty() == false)
		{
			std::vector<Camera*>::iterator it = cams.begin();
			while (it != cams.end())
			{
				(*it)->Draw();
				it++;
			}
		}

		std::vector<GameObject*>::iterator it = childs.begin();
		while (it != childs.end())
		{
			(*it)->Update();
			it++;
		}
	}
}

void GameObject::PostUpdate()
{
	if (active)
	{

		for (std::vector<Component*>::iterator it = components.begin(); it != components.end(); it++)
		{
			(*it)->PostUpdate();
		}
		std::vector<GameObject*>::iterator it = childs.begin();
		while (it != childs.end())
		{
			(*it)->PostUpdate();
			it++;
		}
	}
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
	bool isStatic = IsStatic();
	ImGui::Checkbox("##isObjectStatic", &isStatic);
	if (isStatic != IsStatic() && App->GO->setting == nullptr)
	{
		if (childs.empty() == true)
		{
			SetStatic(isStatic);
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
		else
		{
			color = float4(0, 0.8f, 0.8f, 1);
		}
	}

	App->renderer3D->DrawLocator(float3(0.0f, 0.0f, 0.0f), color);

	if (childs.empty() == false)
	{
		for (std::vector<GameObject*>::iterator it = childs.begin(); it != childs.end(); it++)
		{
			if ((*it)->HasComponent(Component::Type::C_transform) && !(*it)->HasComponent(Component::Type::C_mesh))
			{
				math::float3 childPos((*it)->GetTransform()->GetLocalPos());
				App->renderer3D->DrawLine(float3::zero, childPos, color);
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
	UpdateAABB();
}

void GameObject::UpdateAABB()
{
	aabb.SetNegativeInfinity();
	if (originalAABB.IsFinite())
	{
		OBB obb = originalAABB;
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
	std::vector<Camera*> cams = GetComponent<Camera>();
	if (cams.empty() == false)
	{
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
	if (state == false)
	{
		std::vector<GameObject*>::iterator childIt = childs.begin();
		while (childIt != childs.end())
		{
			(*childIt)->SetActive(state, true);
			childIt++;
		}
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

void GameObject::SetStatic(bool Static)
{
	if (Static != this->Static)
	{
		this->Static = Static;
		if (Static)
		{
			if (parent != nullptr)
			{
				parent->SetStatic(true);
			}
			App->GO->quadTree.Add(this);
		}
		else
		{
			for (std::vector<GameObject*>::iterator it = childs.begin(); it != childs.end(); it++)
			{
				(*it)->SetStatic(false);
			}
			App->GO->quadTree.Remove(this);
		}
	}
}

void GameObject::SetChildsStatic(bool Static)
{
	SetStatic(Static);
	if (Static == true)
	{
		if (childs.empty() == false)
		{
			for (std::vector<GameObject*>::iterator it = childs.begin(); it != childs.end(); it++)
			{
				(*it)->SetChildsStatic(Static);
			}
		}
	}
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
		components.push_back(toAdd);
	}
	return toAdd;
}

bool GameObject::HasComponent(Component::Type type)
{
	if (components.size() > 0)
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
	return false;
}

Transform * GameObject::GetTransform()
{
	return transform;
}
