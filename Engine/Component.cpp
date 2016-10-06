#include "Component.h"

#include "ImGui\imgui.h"

Component::Component(GameObject* linkedTo, int _id)
{
	object = linkedTo;
	id = _id;
	name.create("EmptyComponent");
}

Component::~Component()
{

}

void Component::Enable()
{
	if (enabled == false)
	{
		DoEnable();
		enabled = true;
	}
}

void Component::Disable()
{ 
	if (enabled == true)
	{
		DoDisable();
		enabled = false;
	}
}

void Component::Update()
{
	if (enabled)
	{
		DoUpdate();
	}
}

void Component::DrawOnEditor()
{
	bool active = enabled;
	char _id[56];
	sprintf(_id, "##checkbox%i", id);
	ImGui::Checkbox(_id, &active);
	ImGui::SameLine();
	bool open = ImGui::CollapsingHeader(name.GetString());
	
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

Component::Type Component::GetType()
{
	return type;
}

