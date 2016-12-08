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
	
	// ----------------------- Module Defaults ------------------------------------------------------

	ModuleGoManager(Application* app, bool start_enabled = true);
	~ModuleGoManager();

	bool Init();

	bool Start();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();

	void Render(const viewPort& port);

	bool CleanUp();


	// ----------------------- GO Management ------------------------------------------------------
private:
	GameObject* root = nullptr;
public:
	//Create an empty GameObject
	GameObject* CreateEmpty(const char* name = NULL);
	//Create a gameobject with just a Camera attached to it
	GameObject* CreateCamera(const char* name = NULL);
	//Create a copy of the passed GO
	GameObject* DuplicateGO(GameObject* toCopy);

	//Load a vGO avaliable in the resources
	std::vector<GameObject*> LoadGO(const char* file_noFormat);

	//Delete a GameObject
	bool DeleteGameObject(GameObject* toErase);

	//Set a single GO to the passed Static value
	void SetStatic(bool Static, GameObject* GO);
	//Set a GO and all his childs to the passed Static value
	void SetChildsStatic(bool Static, GameObject* GO);


	// ----------------------- Scene Management ------------------------------------------------------

	void SaveScene(char* name) { wantToSaveScene = true; sceneName = name; }
	void LoadScene(char* name) { wantToLoadScene = true; sceneName = name;	wantToClearScene = true; }
	void ClearScene() { wantToClearScene = true; }

private:
	std::string sceneName;
	bool wantToSaveScene = false;
	bool wantToLoadScene = false;
	bool wantToClearScene = false;
	void SaveSceneNow();
	void LoadSceneNow();
	void ClearSceneNow();
public:

	// ----------------------- UTILITY ------------------------------------------------------

	template <typename C>
	//Returns a vector of all the GOs that collided with the shape passed
	std::vector<GameObject*> FilterCollisions(C col);

	/*Check if the ray collides with any GameObject
	-return bool: wether if the ray collided with something or not
	-OUT_Gameobject: gameobject the ray collided with. If there's none, nullptr is returned
	-OUT_position: the position where the ray collided. If it didn't, it will return (-1,-1).
	-OUT_normal: the direction of the normal of the surface where the ray collided. If it didn't, it will return (-1,-1).*/
	bool RayCast(const LineSegment& ray, GameObject** OUT_gameobject = NULL, float3* OUT_position = NULL, float3* OUT_normal = NULL, bool onlyMeshes = true);

	//Returns the root GO. Only read
	const GameObject* GetRoot() { return root; }

	//Render all the GameObjects onto a viewport.
	//If "exclusiveGOs" vector is empty, all visible GOs will be rendered. Otherwise, only the passed objects will be rendered
	void RenderGOs(const viewPort& viewPort, const std::vector<GameObject*>& exclusiveGOs = std::vector<GameObject*>());
private:
	//Get all the info necessary to render a mesh
	Mesh_RenderInfo GetMeshData(mesh* getFrom);

	void AddGOtoRoot(GameObject* GO);
	void CreateRootGameObject();
	void DeleteGOs();
	std::stack<GameObject*> toDelete;

public:
	//Map with all the components of all the GOs in the scene
	std::multimap<Component::Type, Component*> components;

	// ----------------------- Collision filtering ------------------------------------------------------

	Quad_Tree quadTree;
	bool drawQuadTree = false;
	std::vector<GameObject*> dynamicGO;


	// -------- UI TMP STUFF ------------
	GameObject* setting = nullptr;
	bool settingStatic = true;
private:
	bool StaticChildsPopUpIsOpen = false;
};


template<typename C>
inline std::vector<GameObject*> ModuleGoManager::FilterCollisions(C col)
{

	std::vector<GameObject*> ret = quadTree.FilterCollisions(col);

	for (std::vector<GameObject*>::iterator it = dynamicGO.begin(); it != dynamicGO.end(); it++)
	{
		if ((*it)->IsActive() && (*it)->aabb.IsFinite() && (*it)->aabb.Intersects(col) == true)
		{
			ret.push_back(*it);
		}
	}
	return ret;
}

#endif
