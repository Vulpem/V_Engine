#ifndef  __R_MATERIAL__
#define __R_MATERIAL__

#include "R_Resource.h"
#include "R_Texture.h"
#include "R_Shader.h"
#include "ModuleResourceManager.h"
#include "Mesh_RenderInfo.h"

class R_Material : public Resource
{
public:
	R_Material() :Resource() { alphaTest = 0.2f; }
	R_Material(uint64_t UID) : Resource(UID) { alphaTest = 0.2f; }

	~R_Material()
	{
		if (shader != 0)
		{
			App->resources->UnlinkResource(shader);
		}

		if (textures.empty() == false)
		{
			for (std::vector<uint64_t>::iterator it = textures.begin(); it != textures.end(); it++)
			{
				App->resources->UnlinkResource(*it);
			}
		}
	}

	bool AssignShader(std::string shaderName)
	{
		uint64_t res = App->resources->LinkResource(shaderName, Component::Type::C_Shader);
		if (res == 0) { return false; }
		if (shader != 0)
		{
			App->resources->UnlinkResource(shader);
		}
		shader = res;
	}

	GLuint GetShaderProgram()
	{
		if (shader != 0)
		{			
			return ((R_Shader*)App->resources->Peek(shader))->shaderProgram;
		}
		return App->resources->GetDefaultShader();
	}
	

	Component::Type GetType() { return Component::Type::C_material; }

	float color[5] = { 1.0f, 1.0f, 1.0f,1.0f };

	std::vector<uint64_t> textures;

	uint64_t shader = 0;

	AlphaTestTypes alphaType = AlphaTestTypes::ALPHA_OPAQUE;
	int blendType = GL_ONE_MINUS_SRC_ALPHA;
	float alphaTest;

};

#endif