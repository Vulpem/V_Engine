#pragma once

#include "Component.h"

#include "Math.h"

class Transform : public Component
{
public:
	Transform(GameObject* linkedTo, int id);
	~Transform();
private:
	math::Quat rotation = math::Quat::identity;
	math::float3 position = math::float3::zero;
	math::float3 scale = math::float3::zero;

	void EditorContent();
public:
	math::float4x4 GetTransformMatrix();

	void SetPos(float x, float y, float z);
	void ResetPos();
	math::float3 GetPos();

	void SetRot(float x, float y, float z);
	void SetRot(float x, float y, float z, float w);
	void ResetRot();
	math::float3 GetRot();

	void SetScale(float x, float y, float z);
	void ResetScale();
	math::float3 GetScale();

};