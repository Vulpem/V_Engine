#ifndef __VIEWPORT__
#define __VIEWPORT__

#include "Math.h"
#include "Camera.h"
#include "ModuleWindow.h"

struct viewPort
{
	viewPort(float2 pos, float2 size, Camera* cam, uint ID) :pos(pos), size(size), camera(cam), ID(ID) {};

	bool active = true;
	bool withUI = true;
	bool autoRender = true;

	bool useLighting = true;
	bool useMaterials = true;
	bool useSingleSidedFaces = true;
	bool useOnlyWires = false;

	Camera* camera = nullptr;
	float2 pos = float2::zero;
	float2 size = float2(100,100);

	uint ID = -1;

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
