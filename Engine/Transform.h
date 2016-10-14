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
	math::Quat rotation = math::Quat::identity;
	math::float3 position = math::float3::zero;
	math::float3 scale = math::float3::zero;
	//TMP - Needed to fix the attribute editor display
	math::float3 editorRot = math::float3::zero;

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

	static Type GetType() { return Type::C_transform; }

};

#endif