#include "Component.h"
#include "GameObject.h"

#include "Application.h"
#include "ModuleResourceManager.h"

#include "ImGui\imgui.h"

Component::Component(GameObject* linkedTo, Component::Type type): name("Empty component"), type(type)
{
	object = linkedTo;
	uid = GenerateUUID();
}

Component::~Component()
{
}

void Component::Enable()
{
	if (enabled == false)
	{
		OnEnable();
		enabled = true;
	}
}

void Component::Disable()
{ 
	if (enabled == true)
	{
		OnDisable();
		enabled = false;
	}
}


void Component::DrawOnEditor()
{
	if (GetType() != Component::C_transform)
	{
		bool active = enabled;
		char _id[256];
		sprintf(_id, "##checkbox%llu", uid);
		ImGui::Checkbox(_id, &active);

		if (active != enabled)
		{
			if (active)
			{
				Enable();
			}
			else
			{
				Disable();
			}
		}


		ImGui::SameLine(ImGui::GetWindowWidth() - 50);
		sprintf(_id, "X##RemoveComponent%llu", uid);
		if (ImGui::Button(_id, ImVec2(25, 20)))
		{
			Delete();
		}
		ImGui::SameLine(30);
	}
	bool open = ImGui::CollapsingHeader(name.data());

	if(open && enabled)
	{
		if (MissingComponent() == false)
		{
			EditorContent();
		}
		else
		{
			ImGui::Text("Something went terribly wrong.");
			ImGui::Text("¡This component is missing its resource!");			
		}
	}
}

void Component::Save(pugi::xml_node& myNode)
{
	pugi::xml_node node = myNode.append_child("General");
	char tmpName[256];
	strcpy(tmpName, name.data());
	char* it = tmpName;
	for (int n = 0; n < 255; n++)
	{
		if (it[0] == '#' && it[1] == '#')
		{
			it[0] = '\0';
			break;
		}
		it++;
	}

	node.append_attribute("name") = tmpName;
	node.append_attribute("UID") = uid;
	node.append_attribute("type") = type;
	node.append_attribute("GO") = object->GetUID();
	node.append_attribute("enabled") = IsEnabled();

	SaveSpecifics(myNode.append_child("Specific"));
}

void Component::Delete()
{
	toDelete = true;
}

bool Component::TryDeleteNow()
{
	if (toDelete)
	{
		object->RemoveComponent(this);
		return true;
	}
	return false;
}
