
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

	//position.Set(0, 0, 0);
	//scale.Set(1, 1, 1);
	//rotation = math::Quat::identity;
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

	math::float3 rot = GetRot();
	tmp[0] = rot.x;
	tmp[1] = rot.y;
	tmp[2] = rot.z;
	if (ImGui::DragFloat3("Rotation", tmp, 1.0f))
	{
		for (int n = 0; n < 3; n++)
		{
			while (rot[n] >= 360)
			{
				rot[n] -= 360;
			}
			while (rot[n] < 0)
			{
				rot[n] += 360;
			}
		}

		SetRot(tmp[0], tmp[1], tmp[2]);
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

	math::float4x4 transform = math::float4x4::FromTRS(position, rotation, scale);
	transform.Transpose();
	return transform;
}

void Transform::SetPos(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
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
	x *= DEGTORAD;
	y *= DEGTORAD;
	z *= DEGTORAD;
	if (x == -0) { x = 0; }
	if (y == -0) { y = 0; }
	if (z == -0) { z = 0; }

	rotation = math::Quat::FromEulerXYZ(x, y, z);
}

void Transform::SetRot(float x, float y, float z, float w)
{
	rotation.Set(x, y, z, w);
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
	return ret;
}

void Transform::SetScale(float x, float y, float z)
{
	if (x != 0 && y != 0 && z != 0)
	{
		scale.Set(x, y, z);
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