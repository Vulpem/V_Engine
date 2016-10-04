#ifndef __MODULE_GEOMETRY__
#define __MODULE_GEOMETRY__

#include "Module.h"
#include "Globals.h"

#include "Math.h"
#include <vector>

#define CHECKERS_HEIGHT 128
#define CHECKERS_WIDTH 128

#define NAME_MAX_LEN 1024

struct aiMesh;
struct aiNode;
struct aiScene;

class mesh
{
public:
	char name[NAME_MAX_LEN];

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


	float r = 0.5f;
	float g = 0.5f;
	float b = 0.5f;
	float a = 1.0f;

	uint texture = 0;

	bool wires = false;
	bool selected = false;

	void Draw();
private:
	void RealRender(bool wired = false);
};

class Node
{
public:
	char name[NAME_MAX_LEN];

	std::vector<mesh*> meshes;
	std::vector<Node*> childs;
	Node* parent = nullptr;

	math::Quat rotation = math::Quat::identity;
	math::float3 position = math::float3::zero;
	math::float3 scale = math::float3::zero;

	~Node();

	void Draw();

	void Select();
	void Unselect();

	void SetPos(float x, float y, float z);
	void ResetPos();
	math::float3 GetPos();

	void SetRot(float x, float y, float z);
	void ResetRot();
	math::float3 GetRot();

	void SetScale(float x, float y, float z);
	void ResetScale();
	math::float3 GetScale();

};

class ModuleImportGeometry : public Module
{
public:
	
	ModuleImportGeometry(Application* app, bool start_enabled = true);
	~ModuleImportGeometry();

	bool Init();

	bool Start();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);

	bool CleanUp();

	Node* LoadFBX(char* path);
	bool DeleteNode(Node* toErase);

	uint LoadTexture(char* path);
	uint GetCheckerID() { return id_checkerTexture; }

private:
	Node* LoadNode(const aiNode* toLoad, const aiScene* scene, Node* parent = NULL);
	mesh* LoadMesh(const aiMesh* toLoad, const aiScene* scene);

	void CleanName(char* toClean);
public:
	uint id_checkerTexture;
	std::vector<uint> id_textures;

public:

	std::vector<Node*> geometryNodes;


};

#endif