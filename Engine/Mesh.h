#ifndef __MESH__
#define __MESH__

#include "Globals.h"
#include "Component.h"

struct Mesh_RenderInfo;
struct aiMesh;
struct aiNode;
struct aiScene;
class R_mesh;

class mesh : public  ResourceComponent
{
public:
	mesh(std::string resource, GameObject* linkedTo, int id);
	~mesh();

public:
	int texMaterialIndex = -1;

	bool wires = false;

	Mesh_RenderInfo GetMeshInfo();

	const float3* GetVertices() const;
	const uint* GetIndices() const;
	const float3* GetNormals() const;

	void EditorContent();

	void SaveSpecifics(pugi::xml_node& myNode);

	static Type GetType() { return Type::C_mesh; }
};

#endif