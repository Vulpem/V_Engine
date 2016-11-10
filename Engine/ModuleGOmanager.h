#ifndef __MODULE_GEOMETRY__
#define __MODULE_GEOMETRY__

#include "Module.h"
#include "Globals.h"

#include "Math.h"
#include <vector>
#include <stack>
#include <map>
#include "QuadTree.h"

#include "GameObject.h"

struct viewPort;

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

	void Render(const viewPort& port);

	bool CleanUp();

	GameObject* CreateEmpty(const char* name = NULL);
	GameObject* CreateCamera(const char* name = NULL);
	std::vector<GameObject*> LoadGO(const char* file_noFormat);
	bool DeleteGameObject(GameObject* toErase);

	void SetStatic(bool Static, GameObject* GO);
	void SetChildsStatic(bool Static, GameObject* GO);

	template <typename C>
	std::vector<GameObject*> FilterCollisions(C col);

	const GameObject* GetRoot() { return root; }

private:
	Mesh_RenderInfo GetMeshData(mesh* getFrom);
	void RenderGOs(const viewPort& viewPort);

	void AddGOtoRoot(GameObject* GO);
	void CreateRootGameObject();
	void DeleteGOs();
	std::stack<GameObject*> toDelete;

public:
	std::multimap<Component::Type, Component*> components;
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


template<typename C>
inline std::vector<GameObject*> ModuleGoManager::FilterCollisions(C col)
{

	std::vector<GameObject*> ret = quadTree.FilterCollisions(col);

	for (std::vector<GameObject*>::iterator it = dynamicGO.begin(); it != dynamicGO.end(); it++)
	{
		if ((*it)->aabb.Intersects(col) == true)
		{
			ret.push_back(*it);
		}
	}
	return ret;
}

#endif
