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

void Billboard::UpdateNow(const float3& point, const float3& _up)
{
	if (_up.IsZero() == false)
	{
		Transform* trans = object->GetTransform();
		float3 front = point - trans->GetGlobalPos();

		float4x4 tmp = float4x4::LookAt(localForward.Normalized(), front, localUp.Normalized(), _up);
		trans->SetGlobalRot(tmp.ToEulerXYZ() * RADTODEG);
	}
	else
	{
		object->GetTransform()->LookAt(point);
	}
}

void Billboard::EditorContent()
{
	ImGui::Text("Local Up");
	ImGui::DragFloat3("##DragLocalUp", localUp.ptr(), 0.05f, -1, 1);
	ImGui::Text("Local Forward");
	ImGui::DragFloat3("##DragLocalFront", localForward.ptr(), 0.05f, -1, 1);
}

void Billboard::SaveSpecifics(pugi::xml_node& myNode)
{
}

void Billboard::LoadSpecifics(pugi::xml_node & myNode)
{
}