#ifndef __MODULE_GUI__
#define __MODULE_GUI__

#include "Module.h"
#include "Globals.h"

#include "Math.h"
#include <vector>

struct aiMesh;

struct mesh
{
	uint id_vertices = 0;
	uint num_vertices = 0;
	float* vertices = nullptr;

	uint id_indices = 0;
	uint num_indices = 0;
	uint* indices = nullptr;

	uint id_normals = 0;
	uint num_normals;
	float* normals = nullptr;

	float r = 0.5f;
	float g = 0.5f;
	float b = 0.5f;
	float a = 1.0f;

	math::float4x4 transform = math::float4x4::identity;

	bool wires = false;

	void Draw();

	void SetPos(float x, float y, float z);

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

	mesh* LoadFBX(char* path);

private:
	mesh* LoadMesh(const aiMesh* toLoad, char* path);

public:

	std::vector<mesh*> meshes;

};

#endif