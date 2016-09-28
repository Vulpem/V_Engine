#ifndef __MODULE_GUI__
#define __MODULE_GUI__

#include "Module.h"
#include "Globals.h"

#include <vector>

struct mesh
{
	uint id_vertices = 0;
	uint num_vertices = 0;
	float* vertices = nullptr;

	uint id_indices = 0;
	uint num_indices = 0;
	uint* indices = nullptr;

	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
	float a = 1.0f;

	bool wires = false;

	void Draw();

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

	void LoadFBX(char* path);

	std::vector<mesh*> meshes;

};

#endif