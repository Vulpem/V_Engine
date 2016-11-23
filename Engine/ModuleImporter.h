#ifndef __MODULE_IMPORTER__
#define __MODULE_IMPORTER__

#include "Module.h"
#include "Globals.h"

#include "Math.h"
#include <vector>

struct aiScene;
struct aiMesh;
struct aiNode;
class GameObject;
class Material;

class ModuleImporter : public Module
{
public:
	
	ModuleImporter(Application* app, bool start_enabled = true);
	~ModuleImporter();

	bool Init();

	bool Start();
	bool CleanUp();


// ------------------------------- IMPORTING ------------------------------- 

	//Import everything inside a folder and sub-folders
	void ImportFromFolder(const char* path);

	//Import a specific 3D model and decompose it
	bool Import3dScene(const char* filePath);
	//Import any image to dds
	bool ImportImage(const char* filePath);

private:
	//Import a specific GO. Create a vGO with transform and hierarchy, and call ImportMesh && ImportMaterial
	void ImportGameObject(const char* path, const aiNode* toLoad, const aiScene* scene, bool isChild = false, const char* RootName = nullptr);
	//Create a vmesh from a certain mesh. COntains all mesh info
	std::string ImportMesh(aiMesh* toLoad, const aiScene* scene, const char* name, const char* dir, uint& textureID);
	//Create a vmat from a material, with colors & texture names
	std::string ImportMaterial(const aiScene* scene, std::vector<uint>& matsIndex, const char* matName);
public:



// ------------------------------- LOADING ------------------------------- 


	//The parent variable is for internal use, this is a recursive called function. Please, leave it at NULL, as well as meshesFolder
	GameObject* LoadVgo(const char* fileName, GameObject* parent = nullptr, char* meshesFolder = nullptr);
	void LoadMesh(const char* path, GameObject* toLink);
	void LoadMaterial(const char* path, GameObject* toLink);

	int LoadTexture(char* path, Material* mat);



// ------------------------------- UTILITY ------------------------------- 
	//This function will return only the file format, without the dot.
	std::string FileFormat(const char* file);

	//This function will remove everything from the path except the file's actual name. No format, no path
	std::string FileName(const char* file);

private:
	template <typename type>
	char* CopyMem(char* copyTo, const type* copyFrom, int amount = 1)
	{
		uint bytes = sizeof(type) * amount;
		memcpy(copyTo, copyFrom, bytes);
		return copyTo + bytes;
	}
};

#endif