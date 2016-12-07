#ifndef __MATERIAL__
#define __MATERIAL__

#include "ResourcedComponent.h"
#include "Mesh_RenderInfo.h"

class Material :public ResourcedComponent
{
public:
	Material(std::string res, GameObject* linkedTo);

	void PreUpdate();

	void EditorContent();

	void SaveSpecifics(pugi::xml_node& myNode);
	void LoadSpecifics(pugi::xml_node& myNode);

	uint NofTextures();
	int GetTexture(uint n);

	void SetColor(float r, float g, float b, float a = 1.0f);
	math::float4 GetColor();

	static Type GetType() { return Type::C_material; }

	AlphaTestTypes GetAlphaType();
	void SetAlphaType(AlphaTestTypes type);

	float GetAlphaTest();
	void SetAlphaTest(float alphaTest);

	int GetBlendType();
	void SetBlendType(int blendType);

	std::vector<uint> texturesToRemove;

	void RemoveTexturesNow();
};

#endif