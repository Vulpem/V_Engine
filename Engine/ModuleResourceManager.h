#ifndef __MODULE_RESOURCE_MANAGER__
#define __MODULE_RESOURCE_MANAGER__

#include "Module.h"
#include "Globals.h"

#include "Component.h"

#include <map>

class Resource;
struct Date;

struct MetaInf
{
	std::string name;
	uint64_t uid;
	Component::Type type;
};

struct R_Folder
{
	R_Folder(const char* name, R_Folder* parent);
	R_Folder(const char* name, const char* path);

	std::string name;
	std::string path;

	std::vector<std::string> subFoldersPath;
	std::vector<std::string> files;
};


class ModuleResourceManager : public Module
{
public:
	
	ModuleResourceManager(Application* app, bool start_enabled = true);
	~ModuleResourceManager();

	bool Start();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	//Chooses either we want the files to refresh automatically or not
	bool autoRefresh = true;

	//Number of seconds between file refreshes
	int refreshDelay = 10;
private:
	//Very rudimentary timer to refresh the files automatically
	float refreshTimer = 0.0f;

	//Allows us to find any resource via its UID
	std::map<uint64_t, Resource*> resources;

	//Allows us to find any resource UID via its type and name
	std::map<Component::Type, std::map<std::string, uint64_t>> uidLib;

	//List of the resources we want to Unload
	std::vector<uint64_t> toDelete;

	//List of the resources we want to Reload
	std::vector<uint64_t> toReload;

	//metaData of all files
	//The first map contains the name of the file the resource came from and a map of all the resources linked to that file
	//The second map has all resources from the file ordered by type with its correspondant info
	std::map<std::string, std::multimap<Component::Type,MetaInf>> metaData;

	//Contains the Date each file was last modified
	std::map<std::string, Date> meta_lastMod;

	//R_Folder resBaseFolder;

	Resource* LoadNewResource(std::string fileName, Component::Type type);

public:
	//Check for new or modified files in the Assets folder, and import them. Existing files will keep their UID
	void Refresh();

	//Brute-Force reimport all assets. Existing files won't keep their previous UID
	void ReimportAll();

	//Dump all the library files
	void ClearLibrary();

	//Create the Library folders
	void CreateLibraryDirs();

	//Save metadata for all files
	void SaveMetaData();
	//Save a specific metadata file
	void SaveMetaData(std::map<std::string, std::multimap<Component::Type, MetaInf>>::iterator fileToSave);

	//Read metadata files and load them into memory
	void LoadMetaData();


	const MetaInf* GetMetaData(const char* file, Component::Type type, const char* component);
	const MetaInf* GetMetaData(const char* file, Component::Type type, const uint64_t componentUID);

	//TODO
	//Improve this function, since it's slooooow to find stuff and iterates too much
	const MetaInf* GetMetaData(Component::Type type, const char* component);

private:
	R_Folder ReadFolder(const char* path);
public:
	Resource* Peek(uint64_t uid) const;

	//Link a resource to a new Resourced Component. Use only once per reference
	Resource* LinkResource(uint64_t uid) ;
	uint64_t LinkResource(std::string resName, Component::Type type);

	//Unlink a resource from a Resourced Component. Use only once per reference
	void UnlinkResource(Resource* res);
	void UnlinkResource(uint64_t uid);
	void UnlinkResource(std::string fileName, Component::Type type);

	//Delte the components that need to be deleted
	void DeleteNow();

	//Reload the components that need to be reloaded
	void ReloadNow();

	//Returns all loaded resources. Pretty slow, for debugging use only
	const std::vector<Resource*> ReadLoadedResources() const;

	//Return explanation
	//Returns a vector of pairs:
	// -first is the file name
	// -Second is the vector of resources from that file
	std::vector<std::pair<std::string, std::vector<std::string>>> GetAvaliableResources(Component::Type type = Component::Type::C_None);
};

#endif