
#pragma once
#include "Color.h"

#include "Math.h"

struct Light
{
	Light();

	void Init();
	void SetPos(float x, float y, float z);
	void Active(bool active);
	void Render();

	Color ambient;
	Color diffuse;
	math::float3 position;
	float tmp = 0;

	int ref;
	bool on;
};