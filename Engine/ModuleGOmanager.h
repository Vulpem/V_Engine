#ifndef __MODULE_GEOMETRY__
#define __MODULE_GEOMETRY__

#include "Module.h"
#include "Globals.h"

#include "Math.h"
#include <vector>
#include <stack>
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

	GameObject* CreateEmpty(const char* name = NULL);
	GameObject* CreateCamera(const char* name = NULL);
	std::vector<GameObject*> LoadGO(const char* file_noFormat);
	bool DeleteGameObject(GameObject* toErase);

	std::vector<GameObject*> FilterCollisions(LineSegment col);
	std::vector<GameObject*> FilterCollisions(AABB col);

	const GameObject* GetRoot() { return root; }

private:
	void CreateRootGameObject();
	void DeleteGOs();
	std::stack<GameObject*> toDelete;
public:
	std::vector<uint> id_textures;

	Quad_Tree quadTree;
	bool drawQuadTree = false;
private:
	GameObject* root = nullptr;
public:
	std::vector<GameObject*> dynamicGO;

	//UI TMP STUFF
	GameObject* setting = nullptr;
	bool settingStatic = true;
};

#endif