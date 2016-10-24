
#include "Transform.h"

#include "GameObject.h"

#include "Math.h"

#include "imGUI\imgui.h"


Transform::Transform(GameObject* linkedTo, int id):Component(linkedTo, id)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Transform##%i", id);
	name = tmp;
	type = C_transform;

	localPosition.Set(0, 0, 0);
	localScale.Set(1, 1, 1);
	localRotation = math::Quat::identity;
}
Transform::~Transform()
{

}

void Transform::EditorContent()
{
	float tmp[3];
	tmp[0] = localPosition.x;
	tmp[1] = localPosition.y;
	tmp[2] = localPosition.z;
	if (ImGui::DragFloat3("Position", tmp, 1.0f))
	{
		SetLocalPos(tmp[0], tmp[1], tmp[2]);
	}

	//math::float3 rot = GetLocalRot();
	tmp[0] = editorRot.x;
	tmp[1] = editorRot.y;
	tmp[2] = editorRot.z;
	for (int n = 0; n < 3; n++)
	{
		while (tmp[n] >= 360)
		{
			tmp[n] -= 360;
		}
		while (tmp[n] < 0)
		{
			tmp[n] += 360;
		}
	}
	if (ImGui::DragFloat3("Rotation", tmp, 1.0f))
	{
		SetLocalRot(tmp[0], tmp[1], tmp[2]);
		editorRot.x = tmp[0];
		editorRot.y = tmp[1];
		editorRot.z = tmp[2];
	}

	tmp[0] = localScale.x;
	tmp[1] = localScale.y;
	tmp[2] = localScale.z;
	if (ImGui::DragFloat3("Scale", tmp, 0.01f, 0.1f))
	{
		SetLocalScale(tmp[0], tmp[1], tmp[2]);
	}
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::Text("Global transformations");

	float3 pos = GetGlobalPos();
	ImGui::Text("%.4f, %.4f, %.4f", pos.x, pos.y, pos.z);

	float3 rot = GetGlobalRot();
	ImGui::Text("%f.2, %.4f, %.4f", rot.x, rot.y, rot.z);

	float3 scal = GetGlobalScale();
	ImGui::Text("%f.2, %.4f, %.4f", scal.x, scal.y, scal.z);

}

math::float4x4 Transform::GetLocalTransformMatrix()
{
	if (IsEnabled())
	{
		math::float4x4 transform = math::float4x4::FromTRS(localPosition, localRotation.ToFloat3x3(), localScale);
		transform.Transpose();
		return transform;
	}
	else
	{
		return math::float4x4::identity;
	}
}

void Transform::UpdateGlobalTransform()
{
	if (object->parent && object->parent->HasComponent(Component::Type::C_transform))
	{
		Transform* parent = *object->parent->GetComponent<Transform>().begin();
		
		globalTransform = GetLocalTransformMatrix() * parent->GetGlobalTransform();
	}
	else
	{
		globalTransform = GetLocalTransformMatrix();
	}
}

math::float4x4 Transform::GetGlobalTransform()
{
	return globalTransform;
}

void Transform::UpdateEditorValues()
{
	editorRot = GetLocalRot();
}

void Transform::SetLocalPos(float x, float y, float z)
{
	localPosition.x = x;
	localPosition.y = y;
	localPosition.z = z;

	object->UpdateTransformMatrix();
}

math::float3 Transform::GetLocalPos()
{
	return localPosition;
}

math::float3 Transform::GetGlobalPos()
{
	return globalTransform.Transposed().TranslatePart();
}

void Transform::SetLocalRot(float x, float y, float z)
{
	while (x < 0) { x += 360; }
	while (y < 0) { y += 360; }
	while (z < 0) { z += 360; }

	x *= DEGTORAD;
	y *= DEGTORAD;
	z *= DEGTORAD;

	localRotation = math::Quat::FromEulerXYZ(x, y, z);

	object->UpdateTransformMatrix();
}

void Transform::SetLocalRot(float x, float y, float z, float w)
{
	localRotation.Set(x, y, z, w);

	object->UpdateTransformMatrix();
}

math::float3 Transform::GetLocalRot()
{
	math::float3 ret = localRotation.ToEulerXYZ();
	ret.x *= RADTODEG;
	ret.y *= RADTODEG;
	ret.z *= RADTODEG;

	while (ret.x < 0) { ret.x += 360; }
	while (ret.y < 0) { ret.y += 360; }
	while (ret.z < 0) { ret.z += 360; }

	return ret;
}

math::float3 Transform::GetGlobalRot()
{
	math::float3 ret = globalTransform.ToEulerXYZ();
	ret.x *= RADTODEG;
	ret.y *= RADTODEG;
	ret.z *= RADTODEG;
	return ret;
}

void Transform::SetLocalScale(float x, float y, float z)
{
	if (x != 0 && y != 0 && z != 0)
	{
		localScale.Set(x, y, z);

		object->UpdateTransformMatrix();
	}
}

math::float3 Transform::GetLocalScale()
{
	return localScale;
}

math::float3 Transform::GetGlobalScale()
{
	return globalTransform.ExtractScale();
}
