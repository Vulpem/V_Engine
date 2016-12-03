#ifndef __MODULE_IMPORTER__
#define __MODULE_IMPORTER__

#include "Module.h"
#include "Globals.h"

#include "Component.h"

#include "Math.h"
#include <vector>
#include <map>

struct aiScene;
struct aiMesh;
struct aiNode;
class GameObject;
class Material;
class mesh;
class R_mesh;
class R_Material;
class R_Texture;

struct MetaInf;

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

	//Import a file, without specificating which type of file it is
	std::vector<MetaInf> Import(const char* path);

	//Import a specific 3D model and decompose it
	std::vector<MetaInf> Import3dScene(const char* filePath);
	//Import any image to dds
	std::vector<MetaInf> ImportImage(const char* filePath, uint64_t uid = 0);

private:
	//Import a specific GO. Create a vGO with transform and hierarchy, and call ImportMesh && ImportMaterial
	std::vector<MetaInf> ImportGameObject(const char* path, const aiNode* toLoad, const aiScene* scene);
	//Create a vmesh from a certain mesh. COntains all mesh info
	uint64_t ImportMesh(aiMesh* toLoad, const aiScene* scene, const char* name, uint& textureID);
	//Create a vmat from a material, with colors & texture names
	uint64_t ImportMaterial(const aiScene* scene, std::vector<uint>& matsIndex, const char* matName);

public:

	std::map<std::vector<uint>, uint64_t> tmp_loadedMaterials;

// ------------------------------- LOADING ------------------------------- 


	//The parent variable is for internal use, this is a recursive called function. Please, leave it at NULL, as well as meshesFolder
	GameObject* LoadVgo(const char* fileName, GameObject* parent = nullptr, char* meshesFolder = nullptr);

	R_mesh* LoadMesh(const char* path);
	R_Material* LoadMaterial(const char* path);
	R_Texture* LoadTexture(const char* path);



// ------------------------------- UTILITY ------------------------------- 
	//This function will return only the file format, without the dot.
	std::string FileFormat(const char* file);

	//This function will remove everything from the path except the file's actual name. No format, no path
	std::string FileName(const char* file);

	//Returns the file name + format
	std::string File(const char* file);

	//Will change any "\\" into a "/" && will set the start of the path at "Assets", if found
	std::string NormalizePath(const char* path);

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