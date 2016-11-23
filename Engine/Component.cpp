#include "Component.h"
#include "GameObject.h"

#include "ImGui\imgui.h"

Component::Component(GameObject* linkedTo, int _id): name("Empty component")
{
	object = linkedTo;
	id = _id;
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
	bool active = enabled;
	char _id[56];
	sprintf(_id, "##checkbox%i", id);
	ImGui::Checkbox(_id, &active);
	ImGui::SameLine();
	bool open = ImGui::CollapsingHeader(name.data());
	
	ImGui::SameLine(ImGui::GetWindowWidth() - 60);
	ImGui::Text("ID: %i", id);
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

	if(open && enabled)
	{
		EditorContent();
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
	node.append_attribute("id") = id;
	node.append_attribute("GO") = object->GetUID();
	node.append_attribute("enabled") = enabled;

	SaveSpecifics(myNode.append_child("Specific"));
}


