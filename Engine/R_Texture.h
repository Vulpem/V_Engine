#ifndef  __R_TEXTURE__
#define __R_TEXTURE__

#include "R_Resource.h"
#include "OpenGL.h"

class R_Texture : public Resource
{
public:
	R_Texture() :Resource() { }
	R_Texture(uint64_t UID) : Resource(UID) {}

	~R_Texture()
	{
		glDeleteBuffers(1, &bufferID);
	}

	uint bufferID = 0;

	Component::Type GetType() { return Component::Type::C_Texture; }
};

#endif