#ifndef __BILLBOARD__
#define __BILLBOARD__

#include "ResourcedComponent.h"

class Billboard :public Component
{
public:
	Billboard(GameObject* linkedTo);

	void UpdateNow(const float3& point, const float3& up = float3::zero);

	void EditorContent();

	void SaveSpecifics(pugi::xml_node& myNode);
	void LoadSpecifics(pugi::xml_node& myNode);

	static Type GetType() { return Type::C_Billboard; }
};

#endif