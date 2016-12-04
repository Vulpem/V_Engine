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


	//Import a file, without specificating which type of file it is.
	//The "overWritting" bool will check the resource manager metadata overwrite existing objects instead of creating new ones
	std::vector<MetaInf> Import(const char* path, bool overWritting = false);

	//Import a specific 3D model and decompose it
	std::vector<MetaInf> Import3dScene(const char* filePath, bool overWritting = false);
	//Import any image to dds
	std::vector<MetaInf> ImportImage(const char* filePath, bool overWritting = false);

private:
	//Import a specific GO. Create a vGO with transform and hierarchy, and call ImportMesh && ImportMaterial
	std::vector<MetaInf> ImportGameObject(const char* path, const aiNode* toLoad, const aiScene* scene, uint64_t uid = 0, bool overWriting = false);
	//Create a vmesh from a certain mesh. COntains all mesh info
	uint64_t ImportMesh(aiMesh* toLoad, const aiScene* scene, const char* vGoName, uint& textureID, uint64_t uid = 0);
	//Create a vmat from a material, with colors & texture names
	uint64_t ImportMaterial(const aiScene* scene, std::vector<uint>& matsIndex, const char* matName, uint64_t uid = 0);

public:

	std::map<std::vector<uint>, uint64_t> tmp_loadedMaterials;

// ------------------------------- LOADING ------------------------------- 


	//The parent variable is for internal use, this is a recursive called function. Please, leave it at NULL, as well as meshesFolder
	GameObject* LoadVgo(const char* fileName, const char* vGoName, GameObject* parent = nullptr);

	R_mesh* LoadMesh(const char* resName);
	R_Material* LoadMaterial(const char* resName);
	R_Texture* LoadTexture(const char* resName);



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