#include "Transform.h"

#include "GameObject.h"

#include "Camera.h"

#include "imGUI\imgui.h"


Transform::Transform(GameObject* linkedTo):Component(linkedTo, C_transform)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Transform##%i", uid);
	name = tmp;

	localPosition.Set(0, 0, 0);
	localScale.Set(1, 1, 1);
	localRotation = math::Quat::identity;
}

Transform::~Transform()
{
}

void Transform::SaveSpecifics(pugi::xml_node& myNode)
{
	myNode.append_attribute("AllowRotation") = allowRotation;
	pugi::xml_node node = myNode.append_child("LocalRotation");
	node.append_attribute("x") = localRotation.x;
	node.append_attribute("y") = localRotation.y;
	node.append_attribute("z") = localRotation.z;
	node.append_attribute("w") = localRotation.w;

	node = myNode.append_child("LocalPosition");
	node.append_attribute("x") = localPosition.x;
	node.append_attribute("y") = localPosition.y;
	node.append_attribute("z") = localPosition.z;

	node = myNode.append_child("LocalScale");
	node.append_attribute("x") = localScale.x;
	node.append_attribute("y") = localScale.y;
	node.append_attribute("z") = localScale.z;
}

void Transform::LoadSpecifics(pugi::xml_node & myNode)
{
	allowRotation = myNode.attribute("AllowRotation").as_bool();
	float tmp[4];

	pugi::xml_node rot = myNode.child("LocalRotation");
	tmp[0] = rot.attribute("x").as_float();
	tmp[1] = rot.attribute("y").as_float();
	tmp[2] = rot.attribute("z").as_float();
	tmp[3] = rot.attribute("w").as_float();
	SetLocalRot(tmp[0], tmp[1], tmp[2], tmp[3]);

	pugi::xml_node pos = myNode.child("LocalPosition");
	tmp[0] = pos.attribute("x").as_float();
	tmp[1] = pos.attribute("y").as_float();
	tmp[2] = pos.attribute("z").as_float();
	SetLocalPos(tmp[0], tmp[1], tmp[2]);

	pugi::xml_node scal = myNode.child("LocalScale");
	tmp[0] = scal.attribute("x").as_float();
	tmp[1] = scal.attribute("y").as_float();
	tmp[2] = scal.attribute("z").as_float();
	SetLocalScale(tmp[0], tmp[1], tmp[2]);

}

void Transform::Draw()
{
	if (!object->HasComponent(Component::Type::C_mesh))
	{
		object->DrawLocator();
	}
	//Drawing AABB independantly of object transform
	//REMOVED
	/*if (object->selected)
	{
		object->DrawAABB();
		object->DrawOBB();
	}*/
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
		editorGlobalRot = GetGlobalRot();
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
	if (ImGui::DragFloat3("Global Pos##G_Pos", pos.ptr(), 1.0f))
	{
		SetGlobalPos(pos.x, pos.y, pos.z);
	}

	tmp[0] = editorGlobalRot.x;
	tmp[1] = editorGlobalRot.y;
	tmp[2] = editorGlobalRot.z;
	if (ImGui::DragFloat3("Global Rot##G_Rot", tmp, 1.0f))
	{
		SetGlobalRot(tmp[0], tmp[1], tmp[2]);
		editorGlobalRot.x = tmp[0];
		editorGlobalRot.y = tmp[1];
		editorGlobalRot.z = tmp[2];
		editorRot = GetLocalRot();
	}

	float3 scal = GetGlobalScale();
	ImGui::Text("%.2f, %.2f, %.2f", scal.x, scal.y, scal.z);

}

math::float4x4 Transform::GetLocalTransformMatrix()
{
		math::float4x4 transform = math::float4x4::FromTRS(localPosition, localRotation.ToFloat3x3(), localScale);
		transform.Transpose();
		return transform;
}

void Transform::UpdateGlobalTransform()
{
	if (object->parent != nullptr && object->parent->HasComponent(Component::Type::C_transform) == true)
	{
		Transform* parent = object->parent->GetTransform();
		
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
	editorGlobalRot = GetGlobalRot();
}

void Transform::SetLocalPos(float x, float y, float z)
{
	if (object->IsStatic() == false)
	{
		localPosition.x = x;
		localPosition.y = y;
		localPosition.z = z;

		object->UpdateTransformMatrix();

		if (object->HasComponent(Component::Type::C_camera))
		{
			std::vector<Camera*> cams = object->GetComponent<Camera>();
			std::vector<Camera*>::iterator it = cams.begin();
			while (it != cams.end())
			{
				(*it)->UpdatePos();
				it++;
			}
		}
	}
}

void Transform::SetLocalPos(float3 pos)
{
	SetLocalPos(pos.x, pos.y, pos.z);
}

math::float3 Transform::GetLocalPos()
{
	return localPosition;
}

void Transform::SetGlobalPos(float x, float y, float z)
{
	if (object->IsStatic() == false)
	{
		if (object->parent != nullptr && object->parent->HasComponent(Component::Type::C_transform) == true)
		{
			//TODO
			//Needs cleaning
			Transform* parentTrans = object->parent->GetTransform();

			float4x4 myGlobal = (float4x4::FromTRS(float3(x, y, z), GetGlobalRotQuat(), GetGlobalScale()));
			float4x4 parentGlobal = parentTrans->GetGlobalTransform();

			float4x4 localMat = myGlobal.Transposed() * parentGlobal.Inverted();
			localMat.Transpose();

			SetLocalPos(localMat.TranslatePart().x, localMat.TranslatePart().y, localMat.TranslatePart().z);
		}
		else
		{
			SetLocalPos(x, y, z);
		}
	}
}

void Transform::SetGlobalPos(float3 pos)
{
	SetGlobalPos(pos.x, pos.y, pos.z);
}

math::float3 Transform::GetGlobalPos()
{
	math::float4x4 p = globalTransform.Transposed();
	return p.TranslatePart();
}

void Transform::SetLocalRot(float x, float y, float z)
{
	if (object->IsStatic() == false && allowRotation)
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
}

void Transform::SetLocalRot(float3 rot)
{
	SetLocalRot(rot.x, rot.y, rot.z);
}

void Transform::SetLocalRot(float x, float y, float z, float w)
{
	if (object->IsStatic() == false && allowRotation)
	{
		localRotation.Set(x, y, z, w);

		object->UpdateTransformMatrix();
	}
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

void Transform::SetGlobalRot(float x, float y, float z)
{
	if (object->IsStatic() == false && allowRotation)
	{
		if (object->parent != nullptr && object->parent->HasComponent(Component::Type::C_transform) == true)
		{
			//TODO
			//Needs cleaning
			x *= DEGTORAD;
			y *= DEGTORAD;
			z *= DEGTORAD;

			Transform* parentTrans = object->parent->GetTransform();

			float4x4 localMat = (float4x4::FromTRS(GetGlobalPos(), float3x3::FromEulerXYZ(x, y, z), GetGlobalScale())).Transposed() * parentTrans->GetGlobalTransform().Inverted();
			localMat.Transposed();

			float3 localEuler = localMat.ToEulerXYZ();
			localEuler *= RADTODEG;

			SetLocalRot(localEuler.x, localEuler.y, localEuler.z);
		}
		else
		{
			SetLocalRot(x, y, z);
		}
	}
}

void Transform::SetGlobalRot(float3 rotation)
{
	SetGlobalRot(rotation.x, rotation.y, rotation.z);
}

math::Quat Transform::GetGlobalRotQuat()
{
	float3 pos;
	float3 scal;
	Quat ret;
	globalTransform.Transposed().Decompose(pos, ret, scal);
	return ret;
}

math::float3 Transform::GetGlobalRot()
{
	math::float3 ret = globalTransform.Transposed().ToEulerXYZ();
	ret.x *= RADTODEG;
	ret.y *= RADTODEG;
	ret.z *= RADTODEG;
	return ret;
}

void Transform::SetLocalScale(float x, float y, float z)
{
	if (object->IsStatic() == false)
	{
		if (x != 0 && y != 0 && z != 0)
		{
			localScale.Set(x, y, z);

			object->UpdateTransformMatrix();
		}
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

void Transform::LookAt(const float3 & Spot)
{
	if (object->IsStatic() == false)
	{
		float4x4 tmp = float4x4::LookAt(GetGlobalPos(), Spot, float3(0, 0, 1), float3(0, 1, 0), float3(0, 1, 0));
		SetGlobalRot(tmp.ToEulerXYZ() * RADTODEG);
		UpdateEditorValues();
	}
}



float3 Transform::Up()
{
	return globalTransform.Transposed().WorldY().Normalized();
}

float3 Transform::Down()
{
	return -Up();
}

float3 Transform::Left()
{
	return globalTransform.Transposed().WorldX().Normalized();
}

float3 Transform::Right()
{
	return -Left();
}

float3 Transform::Forward()
{
	return globalTransform.Transposed().WorldZ().Normalized();
}

float3 Transform::Backward()
{
	return -Forward();
}

float3 Transform::WorldUp()
{
	return float3(0,1,0);
}

float3 Transform::WorldDown()
{
	return float3(0, -1, 0);
}

float3 Transform::WorldLeft()
{
	return float3(1, 0, 0);
}

float3 Transform::WorldRight()
{
	return float3(-1, 0, 0);
}

float3 Transform::WorldForward()
{
	return float3(0, 0,1);
}

float3 Transform::WorldBackward()
{
	return float3(0, 0,-1);
}