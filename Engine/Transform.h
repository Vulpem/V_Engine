#ifndef __TRANSFORM__
#define __TRANSFORM__

#include "Component.h"


class math::float3;
class math::Quat;

class Transform : public Component
{
public:
	Transform(GameObject* linkedTo, int id);
	~Transform();
private:
	math::Quat localRotation = math::Quat::identity;
	math::float3 localPosition = math::float3::zero;
	math::float3 localScale = math::float3::zero;
	//TMP - Needed to fix the attribute editor display
	math::float3 editorRot = math::float3::zero;

	math::float4x4 globalTransform = math::float4x4::identity;

	void EditorContent();
public:
	math::float4x4 GetLocalTransformMatrix();

	void UpdateGlobalTransform();
	math::float4x4 GetGlobalTransform();


	void UpdateEditorValues();

	void SetLocalPos(float x, float y, float z);
	math::float3 GetLocalPos();

	math::float3 GetGlobalPos();

	void SetLocalRot(float x, float y, float z);
	void SetLocalRot(float x, float y, float z, float w);
	math::float3 GetLocalRot();

	math::float3 GetGlobalRot();

	void SetLocalScale(float x, float y, float z);
	math::float3 GetLocalScale();

	math::float3 GetGlobalScale();

	static Type GetType() { return Type::C_transform; }

};

#endif