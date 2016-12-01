#ifndef __MESH__
#define __MESH__

#include "Component.h"

struct Mesh_RenderInfo;
class R_mesh;

class mesh : public  ResourceComponent
{
public:
	mesh(std::string resource, GameObject* linkedTo, int id);

public:
	int texMaterialIndex = -1;

	bool wires = false;

	Mesh_RenderInfo GetMeshInfo();

	const float3* GetVertices() const;
	const uint GetNumVertices();
	const uint* GetIndices() const;
	const uint GetNumIndices();
	const float3* GetNormals() const;

	AABB GetAABB();

	void EditorContent();

	void SaveSpecifics(pugi::xml_node& myNode);

	static Type GetType() { return Type::C_mesh; }
};

#endif