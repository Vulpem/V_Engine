#ifndef __TRANSFORM__
#define __TRANSFORM__

#include "Component.h"


class math::float3;
class math::Quat;

class Transform : public Component
{
public:
	Transform(GameObject* linkedTo);
	~Transform();

	bool allowRotation = true;

	void SaveSpecifics(pugi::xml_node& myNode);
	void LoadSpecifics(pugi::xml_node& myNode);
private:
	math::Quat localRotation = math::Quat::identity;
	math::float3 localPosition = math::float3::zero;
	math::float3 localScale = math::float3::zero;
	//TMP - Needed to fix the attribute editor display
	math::float3 editorRot = math::float3::zero;
	math::float3 editorGlobalRot = math::float3::zero;

	math::float4x4 globalTransform = math::float4x4::identity;

	void EditorContent();
public:
	math::float4x4 GetLocalTransformMatrix();

	void UpdateGlobalTransform();
	math::float4x4 GetGlobalTransform();


	void UpdateEditorValues();

	void SetLocalPos(float x, float y, float z);
	void SetLocalPos(float3 pos);
	math::float3 GetLocalPos();

	void SetGlobalPos(float x, float y, float z);
	void SetGlobalPos(float3 pos);
	math::float3 GetGlobalPos();

	void SetLocalRot(float x, float y, float z);
	void SetLocalRot(float3 rot);
	void SetLocalRot(float x, float y, float z, float w);
	math::float3 GetLocalRot();

	void SetGlobalRot(float x, float y, float z);
	void SetGlobalRot(float3 rotation);
	math::Quat GetGlobalRotQuat();
	math::float3 GetGlobalRot();

	void SetLocalScale(float x, float y, float z);
	math::float3 GetLocalScale();

	//void SetGlobalScale(float x, float y, float z);
	math::float3 GetGlobalScale();

	void LookAt(const float3 &Spot);

	float3 Up();
	float3 Down();
	float3 Left();
	float3 Right();
	float3 Forward();
	float3 Backward();

	static float3 WorldUp();
	static float3 WorldDown();
	static float3 WorldLeft();
	static float3 WorldRight();
	static float3 WorldForward();
	static float3 WorldBackward();

	void Draw();

	static Type GetType() { return Type::C_transform; }

};

#endif