
#include "Camera.h"

#include "GameObject.h"
#include "Transform.h"

#include "imGUI\imgui.h"

#include "Math.h"

#include "Application.h"
#include "ModuleGOmanager.h"
#include "ModuleCamera3D.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"


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

	float2 screenSize = App->window->GetWindowSize();
	aspectRatio = (screenSize.x / screenSize.y);

	SetHorizontalFOV(60*DEGTORAD);
	UpdateCamMatrix();
}

Camera::~Camera()
{
	if (App->camera->GetMovingCamera() == this)
	{
		App->camera->SetMovingCamera();
	}
}

void Camera::Draw()
{
	if (object->HiddenFromOutliner() == false)
	{
		DrawFrustum();
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
	frustum.front = object->GetTransform()->GetGlobalTransform().Transposed().WorldZ().Normalized();
}

FrustumCollision Camera::Collides(AABB boundingBox)
{
	float3 points[8];
	boundingBox.GetCornerPoints(points);

	for (int p = 0; p < 6; p++)
	{
		for (int n = 0; n < 8; n++)
		{
			if (frustum.GetPlane(p).IsOnPositiveSide(points[n]) == false)
			{
				break;
			}
			if (n == 7)
			{
				return FrustumCollision::outside;
			}
		}
	}
	return FrustumCollision::contains;
}

FrustumCollision Camera::Collides(float3 point)
{
	Plane planes[6];
	frustum.GetPlanes(planes);

	for (int n = 0; n < 6; n++)
	{
		if (planes[n].IsOnPositiveSide(point) == true)
		{
			return FrustumCollision::outside;
		}
	}
	return FrustumCollision::contains;
}

math::FrustumType Camera::SwitchViewType()
{
	frustumChanged = true;
	if (frustum.type == FrustumType::PerspectiveFrustum)
	{
		otherFOV.x = frustum.horizontalFov;
		otherFOV.y = frustum.verticalFov;
		frustum.type = FrustumType::OrthographicFrustum;
		SetHorizontalFOV(frustum.horizontalFov * 100.0f);
		return FrustumType::OrthographicFrustum;
	}
	frustum.type = FrustumType::PerspectiveFrustum;
	aspectRatio = otherFOV.x / otherFOV.y;
	SetHorizontalFOV(otherFOV.x);
	return FrustumType::PerspectiveFrustum;
}

void Camera::EditorContent()
{
	ImGui::Checkbox("Culling", &hasCulling);
	bool persp = true;
	bool ortho = false;
	if (frustum.type == FrustumType::OrthographicFrustum)
	{
		persp = false;
		ortho = true;
	}
	ImGui::Checkbox("Perspective", &persp);
	ImGui::SameLine();
	ImGui::Checkbox("Orthographic", &ortho);
	if (persp == ortho)
	{
		SwitchViewType();
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
	float maxFOV = 180;
	if (frustum.type == FrustumType::OrthographicFrustum)
	{
		maxFOV = floatMax;
	}
	if (ImGui::DragFloat("Horizontal FOV", &tmp, 1.0f, 1.0f, maxFOV))
	{
		SetHorizontalFOV(tmp * DEGTORAD);
	}
	tmp = frustum.verticalFov * RADTODEG;
	if (ImGui::DragFloat("Vertical FOV", &tmp, 1.0f, 1.0f, maxFOV))
	{
		SetHorizontalFOV(tmp * aspectRatio * DEGTORAD);
	}

	if (ImGui::DragFloat("NearPlane", &frustum.nearPlaneDistance, 0.1f, 0.1f, frustum.farPlaneDistance - 1.0f))
	{
		SetHorizontalFOV(frustum.horizontalFov);
	}
	if (ImGui::DragFloat("FarPlane", &frustum.farPlaneDistance, 1.0f, frustum.nearPlaneDistance + 1.0f, 4000.0f))
	{
		SetHorizontalFOV(frustum.horizontalFov);
	}
}

void Camera::DrawFrustum()
{
	if (object->selected)
	{
		float3 corners[8];
		frustum.GetCornerPoints(corners);
		App->renderer3D->DrawBox(corners);
	}
}

void Camera::SetHorizontalFOV(float horizontalFOV)
{
	frustum.horizontalFov = horizontalFOV;
	frustum.verticalFov = horizontalFOV / aspectRatio;
	frustumChanged = true;
}

void Camera::SetFarPlane(float farPlaneDistance)
{
	if (farPlaneDistance > frustum.nearPlaneDistance)
	{
		frustum.farPlaneDistance = farPlaneDistance;
	}
}

void Camera::SetNearPlane(float nearPlaneDistance)
{
	if (nearPlaneDistance < frustum.farPlaneDistance)
	{
		frustum.nearPlaneDistance = nearPlaneDistance;
	}
}

float4x4 Camera::GetViewMatrix()
{
	return float4x4(frustum.ViewMatrix()).Transposed();
}

float4x4 Camera::GetProjectionMatrix()
{
	return frustum.ProjectionMatrix().Transposed();
}
