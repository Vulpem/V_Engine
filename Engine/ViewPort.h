#ifndef __VIEWPORT__
#define __VIEWPORT__

#include "Math.h"

class Camera;

struct viewPort
{
	Camera* camera = nullptr;
	float2 pos = float2::zero;
	float2 size = float2(100,100);
};


#endif // !__VIEWPORT__
