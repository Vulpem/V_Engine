
#include "Camera.h"

#include "GameObject.h"
#include "Transform.h"

#include "imGUI\imgui.h"

#include "Math.h"

#include "OpenGL.h"

#include "Application.h"
#include "ModuleCamera3D.h"


//------------------------- Camera --------------------------------------------------------------------------------

Camera::Camera(GameObject* linkedTo, int id):Component(linkedTo, id)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Camera##%i", id);
	name = tmp;
	type = C_camera;

	positionOffset = float3::zero;
	if (object)
	{
		if (object->aabb.IsFinite())
		{
			positionOffset = object->aabb.CenterPoint() - object->GetTransform()->GetGlobalPos();
		}
	}
	frustum.nearPlaneDistance = 4;
	frustum.farPlaneDistance = 500;
	frustum.type = FrustumType::PerspectiveFrustum;

	SetHorizontalFOV(20*DEGTORAD);
	UpdateCamMatrix();
}

Camera::~Camera()
{
}

void Camera::DoUpdate()
{
	if (IsEnabled())
	{
	}
}

void Camera::UpdateCamMatrix()
{
	if (object->aabb.IsFinite())
	{
		positionOffset = object->aabb.CenterPoint() - object->GetTransform()->GetGlobalPos();
	}
	UpdateOrientation();
	UpdatePos();
}

void Camera::UpdatePos()
{
	frustum.pos = object->GetTransform()->GetGlobalPos() + positionOffset;
}

void Camera::UpdateOrientation()
{
	float3 rotation = object->GetTransform()->GetGlobalRot();
	rotation *= DEGTORAD;
	float4x4 toSend = float4x4::FromEulerXYZ(rotation.x, rotation.y, rotation.z);
	frustum.SetWorldMatrix(toSend.Float3x4Part());
}

void Camera::EditorContent()
{
	if(ImGui::Button("Set As Active Camera"))
	{
		App->camera->SetActiveCamera(this);
	}
	ImGui::Text("Position:");
	ImGui::Text("X: %.2f, Y: %.2f, Z: %.2f", frustum.pos.x, frustum.pos.y, frustum.pos.z);
	ImGui::Text("Position Offset");
	if (ImGui::DragFloat3("##posOffset", positionOffset.ptr(), 0.1f))
	{
		UpdatePos();
	}

	ImGui::Text("AspectRatio");
	if (ImGui::DragFloat("##Aspect_ratio", &aspectRatio, 0.1f, 0.1f, 5.0f))
	{
		SetHorizontalFOV(frustum.horizontalFov);
	}
	float tmp = frustum.horizontalFov * RADTODEG;
	if (ImGui::DragFloat("Horizontal FOV", &tmp, 1.0f, 1.0f, 180.0f))
	{
		SetHorizontalFOV(tmp * DEGTORAD);
	}
	tmp = frustum.verticalFov * RADTODEG;
	if (ImGui::DragFloat("Vertical FOV", &tmp, 1.0f, 1.0f, 180.0f))
	{
		SetHorizontalFOV(tmp * aspectRatio * DEGTORAD);
	}
	ImGui::DragFloat("NearPlane", &frustum.nearPlaneDistance, 0.1f, 0.1f, frustum.farPlaneDistance - 1.0f);
	ImGui::DragFloat("FarPlane", &frustum.farPlaneDistance, 1.0f, frustum.nearPlaneDistance + 1.0f, 4000.0f);
}

void Camera::Draw()
{
	float3 corners[8];
	frustum.GetCornerPoints(corners);
	object->DrawBox(corners);
}

void Camera::SetHorizontalFOV(float horizontalFOV)
{
	frustum.horizontalFov = horizontalFOV;
	frustum.verticalFov = horizontalFOV / aspectRatio;
	frustumChanged = true;
}
