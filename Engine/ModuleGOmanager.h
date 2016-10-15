#ifndef __MODULE_GEOMETRY__
#define __MODULE_GEOMETRY__

#include "Module.h"
#include "Globals.h"

#include "Math.h"
#include <vector>

#include "GameObject.h"

#define CHECKERS_HEIGHT 128
#define CHECKERS_WIDTH 128

struct aiMesh;
struct aiNode;
struct aiScene;

class GameObject;

//TO_REMOVE
class mesh;

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

	std::vector<GameObject*> LoadGO(const char* file_noFormat);
	bool DeleteGameObject(GameObject* toErase);

private:
	void CreateRootGameObject();
public:
	std::vector<uint> id_textures;

	GameObject* root = NULL;
};

#endif