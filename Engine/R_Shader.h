#ifndef  __R_SHADER__
#define __R_SHADER__

#include "R_Resource.h"
#include "OpenGL.h"

enum ShaderTypes
{
	SHADER_VERTEX = 0,
	SHADER_FRAGMENT,

	//Keep this here
	SHADER_ENUM_END
};

class R_ShaderProgram : public Resource
{
public:
	R_ShaderProgram() :Resource() { }
	R_ShaderProgram(uint64_t UID) : Resource(UID) {}

	~R_ShaderProgram()
	{
		for (uint n = 0; n < SHADER_ENUM_END; n++)
		{
			if (shaders[n] != 0)
			{
				glDeleteShader(shaders[n]);
			}
		}

		if (shaderProgram != 0)
		{
			glDeleteProgram(shaderProgram);
		}
	}

	GLuint shaders[SHADER_ENUM_END];

	GLuint shaderProgram = 0;

	Component::Type GetType() { return Component::Type::C_Shader; }
};

#endif