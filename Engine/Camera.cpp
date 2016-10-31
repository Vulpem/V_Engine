
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
	if (App->camera->GetActiveCamera() == this)
	{
		App->camera->SetCameraToDefault();
	}
}

void Camera::DoPreUpdate()
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
	frustum.front = object->GetTransform()->GetGlobalTransform().Transposed().WorldZ().Normalized();
}

FrustumCollision Camera::Collides(AABB boundingBox)
{
	bool inside = false;
	bool outside = false;

	/*for (int n = 0; n < 6; n++)
	{
		if(frustum.GetPlane(n).Intersects(boundingBox))
		{
			return FrustumCollision::intersects;
		}
	}*/

	for (int n = 0; n < 11; n++)
	{
		if (Collides(boundingBox.Edge(n)) == FrustumCollision::outside)
		{
			outside = true;
		}
		else
		{
			inside = true;
		}
	}

	if (inside == true && outside == false)
	{
		return FrustumCollision::contains;
	}
	else if (outside == true && inside == true)
	{
		return FrustumCollision::intersects;
	}
	return FrustumCollision::outside;
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

FrustumCollision Camera::Collides(math::LineSegment line)
{
	Plane planes[6];
	frustum.GetPlanes(planes);

	for (int n = 0; n < 6; n++)
	{
		if (planes[n].IsOnPositiveSide(line.a) == true)
		{
			if (planes[n].IsOnPositiveSide(line.b) == true)
			{
				return FrustumCollision::outside;
			}
			return FrustumCollision::intersects;
		}
	}
	return FrustumCollision::contains;
}

math::FrustumType Camera::SwitchViewType()
{
	frustumChanged = true;
	if (frustum.type == FrustumType::PerspectiveFrustum)
	{
		frustum.type = FrustumType::OrthographicFrustum;
		return FrustumType::OrthographicFrustum;
	}
	frustum.type = FrustumType::PerspectiveFrustum;
	return FrustumType::PerspectiveFrustum;
}

void Camera::EditorContent()
{
	if(ImGui::Button("Set As Active Camera"))
	{
		App->camera->SetActiveCamera(this);
	}
	if (ImGui::Checkbox("Culling", &hasCulling))
	{
		if (hasCulling == true)
		{
			App->camera->AddCamCulling(this);
		}
		else
		{
			App->camera->RemoveCamCulling(this);
		}
	}
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
	if (App->camera->GetActiveCamera() != this)
	{
		float3 corners[8];
		frustum.GetCornerPoints(corners);
		object->DrawBox(corners);
	}
}

void Camera::SetHorizontalFOV(float horizontalFOV)
{
	frustum.horizontalFov = horizontalFOV;
	frustum.verticalFov = horizontalFOV / aspectRatio;
	frustumChanged = true;
}
