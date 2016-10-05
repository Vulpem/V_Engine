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
		Activate();
		enabled = true;
	}
}

void Component::Disable()
{ 
	if (enabled == true)
	{
		Deactivate();
		enabled = false;
	}
}

void Component::DrawOnEditor()
{
	bool open = ImGui::CollapsingHeader(name.GetString());
	ImGui::SameLine(ImGui::GetWindowWidth() - 60);
	ImGui::Text("Id: %i", id);
	if(open)
	{
		EditorContent();
	}
}

componentType Component::GetType()
{
	return type;
}

