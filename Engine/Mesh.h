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

	//WARNING, the returned value is created via the NEW operator. Remember to destroy it after use
	float3* GetVertices();

	//WARNING, the returned value is created via the NEW operator. Remember to destroy it after use
	uint* GetIndices();

	//WARNING, the returned value is created via the NEW operator. Remember to destroy it after use
	float3* GetNormals();

	void EditorContent();

	void SaveSpecifics(pugi::xml_node myNode);

	static Type GetType() { return Type::C_mesh; }
};

#endif