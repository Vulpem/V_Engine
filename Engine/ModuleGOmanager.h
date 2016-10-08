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

	GameObject* LoadFBX(char* file, bool defaultLocation = true);
	bool DeleteGameObject(GameObject* toErase);

	uint GetCheckerID() { return id_checkerTexture; }

private:
	GameObject* LoadGameObject(const aiNode* toLoad, const aiScene* scene, GameObject* parent = NULL);

	void CleanName(char* toClean);

	void CreateRootGameObject();
public:
	uint id_checkerTexture;
	std::vector<uint> id_textures;

	GameObject* root = NULL;
};

#endif