#pragma once

#include "Component.h"

#include <vector>

class Material :public Component
{
public:
	Material(GameObject* linkedTo, int id);
	~Material();

	void EditorContent();

	int LoadTexture(char* path);
	uint NofTextures();
	uint GetTexture(uint n);

	void SetColor(float r, float g, float b, float a = 1.0f);
	math::float4 GetColor();

private:
	std::vector<uint> textures;
	std::vector<C_String> texturePaths;
	float color[4] = { 0.5f, 0.5f,0.5f,1.0f };

};