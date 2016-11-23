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

	void SaveSpecifics(pugi::xml_node& myNode);
	void LoadSpecifics(pugi::xml_node& myNode);

	uint NofTextures();
	int GetTexture(uint n);

	void SetColor(float r, float g, float b, float a = 1.0f);
	math::float4 GetColor();

	static Type GetType() { return Type::C_material; }

	std::vector<uint> textures;
	std::vector<std::string> texturePaths;
private:
	float color[4] = { 1.0f,1.0f,1.0f,1.0f };

};

#endif