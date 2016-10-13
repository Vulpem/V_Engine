#ifndef __MATERIAL__
#define __MATERIAL__

#include "Component.h"

#include <vector>

class Material :public Component
{
public:
	Material(GameObject* linkedTo, int id);
	~Material();

	void EditorContent();

	uint NofTextures();
	int GetTexture(uint n);

	void SetColor(float r, float g, float b, float a = 1.0f);
	math::float4 GetColor();

	std::vector<uint> textures;
	std::vector<C_String> texturePaths;
private:
	float color[4] = { 1.0f,1.0f,1.0f,1.0f };

};

#endif