#ifndef __CAMERA__
#define __CAMERA__

#include "Globals.h"
#include "Component.h"

enum FrustumCollision
{
	contains,
	intersects,
	outside
};

class Camera : public  Component
{
public:
	Camera(GameObject* linkedTo, int id);
	~Camera();

	void Update();
	void EditorContent();

	void UpdateCamMatrix();
	void UpdatePos();
	void UpdateOrientation();

	FrustumCollision Collides(AABB boundingBox);
	FrustumCollision Collides(float3 point);
	void SetCulling(bool culling) { hasCulling = culling; }
	bool HasCulling() { return hasCulling; }

	math::FrustumType SwitchViewType();

	const math::Frustum* GetFrustum() const { return &frustum; }

	static Type GetType() { return Type::C_camera; }

	void Draw();

	void SetHorizontalFOV(float horizontalFOV);

	float4x4 GetViewMatrix();
	float4x4 GetProjectionMatrix();

	bool active = true;
private:
	math::Frustum frustum;
	bool hasCulling = false;
public:
	bool frustumChanged = true;
	float aspectRatio = 1.4222222f;
	math::float3 positionOffset = float3::zero;
	float2 otherFOV = float2::zero;

};

#endif