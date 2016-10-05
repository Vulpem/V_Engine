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
	if(ImGui::CollapsingHeader(name.GetString()))
	{
		EditorContent();
	}
}

componentType Component::GetType()
{
	return type;
}

