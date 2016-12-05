#ifndef ___MESH_RENDERINFO__
#define ___MESH_RENDERINFO__

#include "Math.h"

enum AlphaTestTypes
{
	ALPHA_OPAQUE = 0,
	ALPHA_DISCARD,
	ALPHA_BLEND
};

struct Mesh_RenderInfo
{
	bool wired = false;
	bool filled = false;
	bool doubleSidedFaces = false;
	bool renderNormals = false;

	float4 meshColor = float4::zero;
	float4 wiresColor = float4::zero;

	unsigned int num_indices = 0;
	unsigned int num_vertices = 0;

	unsigned int indicesBuffer = 0;
	unsigned int vertexBuffer = 0;
	unsigned int textureBuffer = 0;
	unsigned int textureCoordsBuffer = 0;
	unsigned int normalsBuffer = 0;

	AlphaTestTypes alphaType = AlphaTestTypes::ALPHA_OPAQUE;
	int blendType = 0;
	float alphaTest = 0.2f;

	float4x4 transform;
};

#endif // !___MESH_RENDERINFO__

