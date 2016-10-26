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

	static Type GetType() { return Type::C_camera; }

	void Draw();

	bool active = true;
	math::Frustum frustum;
};

#endif