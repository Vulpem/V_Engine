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

	uint id_vertices = 0;
	uint num_vertices = 0;

	uint id_indices = 0;
	uint num_indices = 0;

	uint id_normals = 0;
	uint num_normals;

	uint id_textureCoords = 0;
	uint num_textureCoords = 0;

	int texMaterialIndex = -1;

	bool wires = false;

	Mesh_RenderInfo GetMeshInfo();

	void EditorContent();

	static Type GetType() { return Type::C_mesh; }
};

#endif