#ifndef __MODULE_GEOMETRY__
#define __MODULE_GEOMETRY__

#include "Module.h"
#include "Globals.h"

#include "Math.h"
#include <vector>
#include "QuadTree.h"

#include "GameObject.h"


class GameObject;

class ModuleGoManager : public Module
{
public:
	
	ModuleGoManager(Application* app, bool start_enabled = true);
	~ModuleGoManager();

	bool Init();

	bool Start();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);

	bool CleanUp();

	GameObject* CreateEmpty();
	GameObject* CreateCamera();
	std::vector<GameObject*> LoadGO(const char* file_noFormat);
	bool DeleteGameObject(GameObject* toErase);

private:
	void CreateRootGameObject();
public:
	std::vector<uint> id_textures;

	Quad_Tree quadTree;

	GameObject* root = nullptr;
};

#endif