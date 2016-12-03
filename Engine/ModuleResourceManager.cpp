#include "Globals.h"
#include "Application.h"

#include "ModuleResourceManager.h"

#include "ModuleImporter.h"
#include "ModuleFileSystem.h"

#include "R_Resource.h"

#include <queue>


ModuleResourceManager::ModuleResourceManager(Application* app, bool start_enabled) : Module(app, start_enabled)//, resBaseFolder("Assets", "Assets")
{
	name = "ModuleResourceManager";
}

// Destructor
ModuleResourceManager::~ModuleResourceManager()
{
}

// Called before render is available
bool ModuleResourceManager::Start()
{
	CreateLibraryDirs();

	return true;
}

// Called every draw update
update_status ModuleResourceManager::PreUpdate()
{

	return UPDATE_CONTINUE;
}

update_status ModuleResourceManager::PostUpdate()
{
	DeleteNow();
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleResourceManager::CleanUp()
{
	std::map<uint64_t, Resource*>::iterator it = resources.begin();
	for (; it != resources.end(); it++)
	{
		RELEASE(it->second);
	}

	resources.clear();
	uidLib.clear();

	return true;
}

Resource * ModuleResourceManager::LoadNewResource(std::string fileName)
{
	std::string format(".");
	format += App->importer->FileFormat(fileName.data());
	if (format == MESH_FORMAT)
	{
		return (Resource*)App->importer->LoadMesh(fileName.data());
	}	
	if (format == MATERIAL_FORMAT)
	{
		return (Resource*)App->importer->LoadMaterial(fileName.data());
	}
	if (format == TEXTURE_FORMAT)
	{
		return (Resource*)App->importer->LoadTexture(fileName.data());
	}
	return nullptr;
}

void ModuleResourceManager::CreateLibraryDirs()
{
	App->fs->CreateDir("Library");
	App->fs->CreateDir("Library/Meshes");
	App->fs->CreateDir("Library/Textures");
	App->fs->CreateDir("Library/vGOs");
	App->fs->CreateDir("Library/Materials");
	App->fs->CreateDir("Library/Meta");
	App->fs->CreateDir("Assets/Scenes");
}

void ModuleResourceManager::ReimportAll()
{
	ClearLibrary();

	std::queue<R_Folder> pendant;
	pendant.push(ReadFolder("Assets"));
	while (pendant.empty() == false)
	{
		for (std::vector<std::string>::iterator it = pendant.front().subFoldersPath.begin(); it != pendant.front().subFoldersPath.end(); it++)
		{
			pendant.push(ReadFolder(it->data()));
		}

		for (std::vector<std::string>::iterator it = pendant.front().files.begin(); it != pendant.front().files.end(); it++)
		{
			std::string path(pendant.front().path);
			path += "/";
			path += it->data();

			std::vector<MetaInf> toAdd = App->importer->Import(path.data());
			if (toAdd.empty() == false)
			{
				std::multimap<Component::Type, MetaInf> tmp;
				for (std::vector<MetaInf>::iterator m = toAdd.begin(); m != toAdd.end(); m++)
				{
					tmp.insert(std::pair<Component::Type, MetaInf>(m->type, *m));
				}
				metaData.insert(std::pair<std::string, std::multimap<Component::Type, MetaInf>>(path, tmp));

				meta_lastMod.insert(std::pair<std::string, Date>(path, App->fs->ReadFileDate(path.data())));
			}
		}

		pendant.pop();
	}

	SaveMetaData();
}

void ModuleResourceManager::ClearLibrary()
{
	App->fs->DelDir("Library");
	CreateLibraryDirs();
}

void ModuleResourceManager::SaveMetaData()
{
	std::map<std::string, std::multimap<Component::Type, MetaInf>>::iterator fileIt = metaData.begin();
	for (; fileIt != metaData.end(); fileIt++)
	{
		pugi::xml_document data;
		pugi::xml_node root_node;

		std::string fileName("Library/Meta/");
		fileName += App->importer->FileName(fileIt->first.data());
		fileName += META_FORMAT;

		root_node = data.append_child("File");

		pugi::xml_node fileData = root_node.append_child("FileData");
		fileData.append_attribute("name") = fileIt->first.data();

		std::map<std::string, Date>::iterator date = meta_lastMod.find(fileIt->first);

		fileData.append_attribute("year") = date->second.year;
		fileData.append_attribute("month") = date->second.month;
		fileData.append_attribute("day") = date->second.day;
		fileData.append_attribute("hour") = date->second.hour;
		fileData.append_attribute("min") = date->second.min;
		fileData.append_attribute("sec") = date->second.sec;

		std::multimap<Component::Type, MetaInf>::iterator it = fileIt->second.begin();
		for (; it != fileIt->second.end(); it++)
		{
			pugi::xml_node link = root_node.append_child("link");
			link.append_attribute("name") = it->second.name.data();
			link.append_attribute("type") = it->second.type;
			link.append_attribute("uid") = it->second.uid;
		}

		std::stringstream stream;
		data.save(stream);
		// we are done, so write data to disk
		App->fs->Save(fileName.data(), stream.str().c_str(), stream.str().length());
		LOG("Created: %s", fileName.data());

		data.reset();
	}
}

const MetaInf* ModuleResourceManager::GetMetaData(const char * file, Component::Type type, const char * component)
{
	std::map<std::string, std::multimap<Component::Type, MetaInf>>::iterator f = metaData.find(file);
	if (f != metaData.end())
	{
		std::multimap<Component::Type, MetaInf> ::iterator it = f->second.find(type);
		while (it->first == type && it != f->second.end())
		{
			if (it->second.name.compare(component) == 0)
			{
				return &it->second;
			}
			it++;
		}
	}
	return nullptr;
}

/*
void ModuleResourceManager::RefreshFolder(const char * path)
{
	R_Folder meta = ReadFolderMeta(path);
	R_Folder real = ReadFolder(path);

	for (std::vector<std::string>::iterator metaIt = real.subFoldersPath.begin(); metaIt != real.subFoldersPath.end(); metaIt++)
	{
		for (std::vector<std::string>::iterator realIt = meta.subFoldersPath.begin(); realIt != meta.subFoldersPath.end(); realIt++)
		{

		}
	}

	for (std::vector<std::string>::iterator it = real.subFoldersPath.begin(); it != real.subFoldersPath.end(); it++)
	{

	}

}
*/
R_Folder ModuleResourceManager::ReadFolder(const char * path)
{
	R_Folder ret(App->importer->FileName(path).data(), path);

	std::vector<std::string> folders;
	std::vector<std::string> files;
	App->fs->GetFilesIn(path, &folders, &files);

	for (std::vector<std::string>::iterator it = folders.begin(); it != folders.end(); it++)
	{
		std::string _path(path);
		_path += "/";
		_path += it->data();
		ret.subFoldersPath.push_back(_path);
	}
	for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); it++)
	{
		ret.files.push_back(it->data());
	}

	return ret;
}

/*void ModuleResourceManager::CreateFolderMeta(R_Folder & folder)
{
	pugi::xml_document data;
	pugi::xml_node root_node;

	std::string fileName(folder.path);
	fileName += META_FORMAT;

	root_node = data.append_child("folder");

	root_node.append_attribute("name") = folder.name.data();

	for (std::vector<std::string>::iterator it = folder.subFoldersPath.begin(); it != folder.subFoldersPath.end(); it++)
	{
		pugi::xml_node sub = root_node.append_child("sub");
		sub.append_attribute("data") = it->data();
	}

	for (std::vector<std::string>::iterator it = folder.files.begin(); it != folder.files.end(); it++)
	{
		pugi::xml_node sub = root_node.append_child("file");
		sub.append_attribute("data") = it->data();
	}

	std::stringstream stream;
	data.save(stream);
	// we are done, so write data to disk
	App->fs->Save(fileName.data(), stream.str().c_str(), stream.str().length());
	LOG("Created: %s", fileName.data());

	data.reset();
}
*/
/*R_Folder ModuleResourceManager::ReadFolderMeta(const char * path)
{
	char* buffer;
	std::string _path = path;
	_path += META_FORMAT;
	uint size = App->fs->Load(_path.data(), &buffer);

	if (size > 0)
	{
		pugi::xml_document data;
		pugi::xml_node root;

		pugi::xml_parse_result result = data.load_buffer(buffer, size);
		RELEASE(buffer);

		if (result != NULL)
		{
			root = data.child("folder");
			if (root)
			{
				std::string name = root.attribute("name").as_string();

				R_Folder ret(name.data(), path);

				for (pugi::xml_node folder = root.child("sub"); folder != nullptr; folder = folder.next_sibling("folder"))
				{
					ret.subFoldersPath.push_back(folder.attribute("data").as_string());
				}

				for (pugi::xml_node files = root.child("file"); files != nullptr; files = files.next_sibling("file"))
				{
					ret.subFoldersPath.push_back(files.attribute("data").as_string());
				}

				return ret;
			}
		}
	}
	LOG("Tried to read an unexisting folder meta.\n%s", path)
	return R_Folder("", "");
}*/

Resource * ModuleResourceManager::Peek(uint64_t uid) const
{
	std::map<uint64_t, Resource*>::const_iterator it = resources.find(uid);
	if (it != resources.end())
	{
		return it->second;
	}
	return nullptr;
}

Resource * ModuleResourceManager::LinkResource(uint64_t uid)
{
	Resource* ret = nullptr;
	std::map<uint64_t, Resource*>::iterator it = resources.find(uid);
	if (it != resources.end())
	{
		ret = it->second;
		ret->nReferences++;
	}
	return ret;
}

uint64_t ModuleResourceManager::LinkResource(std::string fileName, Component::Type type)
{
	Resource* ret = nullptr;
	std::map<Component::Type, std::map<std::string, uint64_t>>::iterator tmpMap = uidLib.find(type);
	if (tmpMap == uidLib.end())
	{
		uidLib.insert(std::pair<Component::Type, std::map<std::string, uint64_t>>(type, std::map<std::string, uint64_t>()));
		tmpMap = uidLib.find(type);
	}
	std::map<std::string, uint64_t> ::iterator it = tmpMap->second.find(fileName);

	if (it != tmpMap->second.end())
	{
		ret = LinkResource(it->second);
	}

	if (ret == nullptr)
	{
		ret = LoadNewResource(fileName);
		if (ret != nullptr)
		{
			resources.insert(std::pair<uint64_t, Resource*>(ret->uid, ret));
			tmpMap->second.insert(std::pair<std::string, uint64_t>(ret->file, ret->uid));
			ret->nReferences++;
		}
	}

	if (ret != nullptr)
	{
		return ret->uid;
	}
	return 0;
}

void ModuleResourceManager::UnlinkResource(Resource * res)
{
	UnlinkResource(res->uid);
}

void ModuleResourceManager::UnlinkResource(uint64_t uid)
{
	std::map<uint64_t, Resource*>::iterator it = resources.find(uid);
	if (it != resources.end() && it->second != nullptr)
	{
		it->second->nReferences--;
		if (it->second->nReferences <= 0)
		{
			toDelete.push_back(it->first);
		}
	}
}

void ModuleResourceManager::UnlinkResource(std::string fileName, Component::Type type)
{
	std::map<Component::Type, std::map<std::string, uint64_t>>::iterator tmpMap = uidLib.find(type);
	if (tmpMap != uidLib.end())
	{
		std::map<std::string, uint64_t>::iterator it = tmpMap->second.find(fileName);
		if (it != tmpMap->second.end())
		{
			UnlinkResource(it->second);
		}
	}
}

void ModuleResourceManager::DeleteNow()
{
	if (toDelete.empty() == false)
	{
		std::vector<uint64_t> tmp = toDelete;
		toDelete.clear();
		while (tmp.empty() == false)
		{
			uint64_t uid = tmp.back();

			//Erasing the resource itself
			std::map<uint64_t, Resource*>::iterator it = resources.find(uid);
			if (it != resources.end())
			{
				RELEASE(it->second);
				resources.erase(it);
			}

			//TODO
			//When metadata is created, we won't erase from here anymore

			/*bool found = false;
			//Erasing its reference in the uid Lib
			std::map<Component::Type, std::map<std::string, uint64_t>>::iterator tmpMap = uidLib.begin();
			for (; tmpMap != uidLib.end() && found == false; tmpMap++)
			{
				std::map<std::string, uint64_t>::iterator it = tmpMap->second.begin();
				for (;it != tmpMap->second.end() && found == false; it++)
				{
					if (it->second == uid)
					{
						tmpMap->second.erase(it);
						found = true;
					}
				}
			}*/
			tmp.pop_back();
		}
	}
}

const std::vector<Resource*> ModuleResourceManager::ReadLoadedResources() const
{
	std::vector<Resource*> ret;

	std::map<Component::Type, std::map<std::string, uint64_t>>::const_iterator tmpMap = uidLib.cbegin();
	for (; tmpMap != uidLib.end(); tmpMap++)
	{
		std::map<std::string, uint64_t>::const_iterator it = tmpMap->second.cbegin();
		for (; it != tmpMap->second.end(); it++)
		{
			std::map<uint64_t, Resource*>::const_iterator res = resources.find(it->second);
			if (res != resources.cend())
			{
				ret.push_back(res->second);
			}
		}
	}

	return ret;
}

R_Folder::R_Folder(const char* name, R_Folder* parent) : name(name)
{
	if (parent != nullptr)
	{
		std::string myPath(parent->path);
		myPath += "/";
		myPath += name;
		path = myPath;
	}
	else
	{
		path = name;
	}
}

R_Folder::R_Folder(const char * name, const char * path) : name(name), path(path)
{
}
