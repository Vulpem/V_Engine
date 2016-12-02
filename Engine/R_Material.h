#ifndef  __R_MATERIAL__
#define __R_MATERIAL__

#include "R_Resource.h"
#include "R_Texture.h"
#include "ModuleResourceManager.h"

class R_Material : public Resource
{
public:
	R_Material() :Resource() { }
	R_Material(uint64_t UID) : Resource(UID) {}

	~R_Material()
	{
		if (textures.empty() == false)
		{
			for (std::vector<uint64_t>::iterator it = textures.begin(); it != textures.end(); it++)
			{
				App->resources->UnlinkResource(*it);
			}
		}
	}

	Component::Type GetType() { return Component::Type::C_material; }

	float color[5] = { 1.0f, 1.0f, 1.0f,1.0f };

	std::vector<uint64_t> textures;

};

#endif