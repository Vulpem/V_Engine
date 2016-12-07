#include "Billboard.h"

#include "GameObject.h"
#include "Transform.h"
#include "Application.h"

#include "imGUI\imgui.h"

Billboard::Billboard(GameObject* linkedTo) : Component(linkedTo, C_Billboard)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Billboard##%i", uid);
	name = tmp;
}

void Billboard::UpdateNow(const float3& point, const float3& up)
{
	if (up.IsZero() == false)
	{
		/*Transform* trans = object->GetTransform();

		float3 dir = point - trans->GetGlobalPos();


		float4x4 tmp = float4x4::LookAt(GetGlobalPos(), Spot, float3(0, 0, 1), float3(0, 1, 0), float3(0, 1, 0));
		SetGlobalRot(tmp.ToEulerXYZ() * RADTODEG);
		UpdateEditorValues();*/
	}
	else
	{
		object->GetTransform()->LookAt(point);
	}
}

void Billboard::EditorContent()
{
	ImGui::Text("I'm a billboard!");
	ImGui::Text("Nice to meet you!");
}

void Billboard::SaveSpecifics(pugi::xml_node& myNode)
{
}

void Billboard::LoadSpecifics(pugi::xml_node & myNode)
{
}