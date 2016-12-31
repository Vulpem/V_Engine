#ifndef ___MESH_RENDERINFO__
#define ___MESH_RENDERINFO__

#include "Math.h"

class R_mesh;

enum AlphaTestTypes
{
	ALPHA_OPAQUE = 0,
	ALPHA_DISCARD,
	ALPHA_BLEND
};

struct Mesh_RenderInfo
{
	const R_mesh* origin = nullptr;

	bool wired = false;
	bool filled = false;
	bool doubleSidedFaces = false;
	bool renderNormals = false;

	bool hasNormals = false;
	bool hasUVs = false;

	float4 meshColor = float4::zero;
	float4 wiresColor = float4::zero;

	unsigned int num_indices = 0;
	unsigned int num_vertices = 0;

	unsigned int indicesBuffer = 0;
	unsigned int dataBuffer = 0;
	unsigned int textureBuffer = 0;

	AlphaTestTypes alphaType = AlphaTestTypes::ALPHA_OPAQUE;
	int blendType = 0;
	float alphaTest = 0.2f;

	uint shader = 0;

	float4x4 transform;
};

#endif // !___MESH_RENDERINFO__

