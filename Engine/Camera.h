#ifndef __CAMERA__
#define __CAMERA__

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
	Camera(GameObject* linkedTo);
	~Camera();

	void PreUpdate();

	void EditorContent();

	void SaveSpecifics(pugi::xml_node& myNode);
	void LoadSpecifics(pugi::xml_node& myNode);

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
	void DrawFrustum();

	void SetHorizontalFOV(float horizontalFOV);
	void SetFarPlane(float farPlaneDistance);
	void SetNearPlane(float nearPlaneDistance);

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