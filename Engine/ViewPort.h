#ifndef __VIEWPORT__
#define __VIEWPORT__

#include "Math.h"
#include "Camera.h"
#include "ModuleWindow.h"

struct viewPort
{
	viewPort(float2 pos, float2 size, Camera* cam) :pos(pos), size(size), camera(cam){};

	bool active = true;

	Camera* camera = nullptr;
	float2 pos = float2::zero;
	float2 size = float2(100,100);

	void SetCameraMatrix()
	{
		if (camera != nullptr)
		{
			camera->aspectRatio = size.x / size.y;
			camera->SetHorizontalFOV(camera->GetFrustum()->horizontalFov);
		}
	}

};


#endif // !__VIEWPORT__
