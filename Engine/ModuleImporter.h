#ifndef __MODULE_IMPORTER__
#define __MODULE_IMPORTER__

#include "Module.h"
#include "Globals.h"

#include "Math.h"
#include <vector>

class aiScene;
class aiNode;

class ModuleImporter : public Module
{
public:
	
	ModuleImporter(Application* app, bool start_enabled = true);
	~ModuleImporter();

	bool Init();

	bool Start();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);

	void ImportFromFolder(const char* path);
	void Import3dScene(const char* filePath);

private:
	void ImportGameObject(const char* path, const aiNode* toLoad, const aiScene* scene, bool isChild = false, const char* RootName = NULL);

public:
	void CleanName(char* toClean);
	std::string FileFormat(const char* file);
	std::string FileName(const char* file);

	bool CleanUp();
};

#endif