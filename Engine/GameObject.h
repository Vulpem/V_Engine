#pragma once

#define NAME_MAX_LEN 1024

#include "Globals.h"

#include "Mesh.h"

#include<vector>
#include "Math.h"



class GameObject
{
public:
	char name[NAME_MAX_LEN];

	std::vector<mesh*> meshes;
	std::vector<GameObject*> childs;
	GameObject* parent = nullptr;

	math::Quat rotation = math::Quat::identity;
	math::float3 position = math::float3::zero;
	math::float3 scale = math::float3::zero;

	~GameObject();

	void Draw();

	void Select();
	void Unselect();

	void SetPos(float x, float y, float z);
	void ResetPos();
	math::float3 GetPos();

	void SetRot(float x, float y, float z);
	void ResetRot();
	math::float3 GetRot();

	void SetScale(float x, float y, float z);
	void ResetScale();
	math::float3 GetScale();

};