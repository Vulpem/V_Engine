#include "Component.h"

#include "ImGui\imgui.h"

Component::Component(GameObject* linkedTo, int _id): name("Empty component")
{
	object = linkedTo;
	id = _id;
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

