#ifndef  __R_SHADER__
#define __R_SHADER__

#include "R_Resource.h"
#include "ModuleResourceManager.h"
#include "R_Texture.h"

class R_Shader : public Resource
{
public:
	R_Shader() :Resource() { }
	R_Shader(uint64_t UID) : Resource(UID) { }

	~R_Shader()
	{
		if (shaderProgram != 0)
		{
			glDeleteProgram(shaderProgram);
		}
	}

	Component::Type GetType() { return Component::Type::C_Shader; }

	GLuint shaderProgram = 0;
};

#endif