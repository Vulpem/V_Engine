#ifndef __MESH__
#define __MESH__

#include "Globals.h"
#include "Component.h"

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
	float* vertices = nullptr;

	uint id_indices = 0;
	uint num_indices = 0;
	uint* indices = nullptr;

	uint id_normals = 0;
	uint num_normals;
	float* normals = nullptr;

	uint id_textureCoords = 0;
	uint num_textureCoords = 0;
	float* textureCoords = nullptr;

	int texMaterialIndex = -1;

	bool init = false;

	bool wires = false;

	void DoUpdate();
	void EditorContent();
	bool LoadMesh(const aiMesh* toLoad, const aiScene* scene);

	void Draw();
private:
	void RealRender(bool wired = false);
	void RenderNormals();
};

#endif