#ifndef __CAMERA__
#define __CAMERA__

#include "Globals.h"
#include "Component.h"

class Camera : public  Component
{
public:
	Camera(GameObject* linkedTo, int id);
	~Camera();

	void DoUpdate();
	void EditorContent();

	void UpdateCamMatrix();
	void UpdatePos();
	void UpdateOrientation();

	const math::Frustum* GetFrustum() const { return &frustum; }

	static Type GetType() { return Type::C_camera; }

	void Draw();

	void SetHorizontalFOV(float horizontalFOV);

	bool active = true;
private:
	math::Frustum frustum;
public:
	bool frustumChanged = true;
	math::FrustumType viewType = math::FrustumType::PerspectiveFrustum;
	float aspectRatio = 1.77777777f;
	math::float3 positionOffset = float3::zero;
};

#endif