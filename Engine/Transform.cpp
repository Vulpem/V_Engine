
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

	position.Set(0, 0, 0);
	scale.Set(1, 1, 1);
	rotation = math::Quat::identity;
}
Transform::~Transform()
{

}

void Transform::EditorContent()
{
	float tmp[3];
	tmp[0] = position.x;
	tmp[1] = position.y;
	tmp[2] = position.z;
	if (ImGui::DragFloat3("Position", tmp, 1.0f))
	{
		SetPos(tmp[0], tmp[1], tmp[2]);
	}

	//math::float3 rot = GetRot();
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
		SetRot(tmp[0], tmp[1], tmp[2]);
		editorRot.x = tmp[0];
		editorRot.y = tmp[1];
		editorRot.z = tmp[2];
	}

	tmp[0] = scale.x;
	tmp[1] = scale.y;
	tmp[2] = scale.z;
	if (ImGui::DragFloat3("Scale", tmp, 0.01f, 0.1f))
	{
		SetScale(tmp[0], tmp[1], tmp[2]);
	}
}

math::float4x4 Transform::GetTransformMatrix()
{
	if (IsEnabled())
	{
		math::float4x4 transform = math::float4x4::FromTRS(position, rotation.ToFloat3x3(), scale);
		transform.Transpose();
		return transform;
	}
	else
	{
		return math::float4x4::identity;
	}
}

void Transform::UpdateEditorValues()
{
	editorRot = GetRot();
}

void Transform::SetPos(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;

	object->UpdateAABB();
}

void Transform::ResetPos()
{
	SetPos(0, 0, 0);
}

math::float3 Transform::GetPos()
{
	return position;
}

void Transform::SetRot(float x, float y, float z)
{
	while (x < 0) { x += 360; }
	while (y < 0) { y += 360; }
	while (z < 0) { z += 360; }

	x *= DEGTORAD;
	y *= DEGTORAD;
	z *= DEGTORAD;

	rotation = math::Quat::FromEulerXYZ(x, y, z);

	object->UpdateAABB();
}

void Transform::SetRot(float x, float y, float z, float w)
{
	rotation.Set(x, y, z, w);

	object->UpdateAABB();
}

void Transform::ResetRot()
{
	SetRot(0, 0, 0);
}

math::float3 Transform::GetRot()
{
	math::float3 ret = rotation.ToEulerXYZ();
	ret.x *= RADTODEG;
	ret.y *= RADTODEG;
	ret.z *= RADTODEG;

	while (ret.x < 0) { ret.x += 360; }
	while (ret.y < 0) { ret.y += 360; }
	while (ret.z < 0) { ret.z += 360; }

	return ret;
}

void Transform::SetScale(float x, float y, float z)
{
	if (x != 0 && y != 0 && z != 0)
	{
		scale.Set(x, y, z);

		object->UpdateAABB();
	}
}

void Transform::ResetScale()
{
	SetScale(1, 1, 1);
}

math::float3 Transform::GetScale()
{
	return scale;
}