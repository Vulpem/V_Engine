#ifndef __MESH__
#define __MESH__

#include "Globals.h"
#include "Component.h"

struct Mesh_RenderInfo;
struct aiMesh;
struct aiNode;
struct aiScene;

class mesh : public  Component
{
public:
	mesh(GameObject* linkedTo, int id);
	~mesh();

	std::string meshPath;

	float3* vertices = nullptr;
	uint id_vertices = 0;
	uint num_vertices = 0;

	uint* indices = nullptr;
	uint id_indices = 0;
	uint num_indices = 0;

	float3* normals = nullptr;
	uint id_normals = 0;
	uint num_normals;

	uint id_textureCoords = 0;
	uint num_textureCoords = 0;

	int texMaterialIndex = -1;

	AABB aabb;

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