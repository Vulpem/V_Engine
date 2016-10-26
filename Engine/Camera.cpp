
#include "Camera.h"

#include "GameObject.h"

#include "imGUI\imgui.h"

#include "Math.h"


//------------------------- Camera --------------------------------------------------------------------------------

Camera::Camera(GameObject* linkedTo, int id):Component(linkedTo, id)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Camera##%i", id);
	name = tmp;
	type = C_camera;
	//frustum.SetWorldMatrix()
}

Camera::~Camera()
{
}

void Camera::DoUpdate()
{
	if (IsEnabled())
	{
		Draw();
	}
}

void Camera::EditorContent()
{
	
}

void Camera::Draw()
{

}