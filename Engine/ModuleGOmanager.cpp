#include "Globals.h"
#include "Application.h"

#include "ModuleGOmanager.h"

#include "ModuleInput.h"
#include "ModuleImporter.h"

#include "AllComponents.h"



//------------------------- MODULE --------------------------------------------------------------------------------

ModuleGoManager::ModuleGoManager(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name.create("ModuleGeometry");
}

// DestructorF
ModuleGoManager::~ModuleGoManager()
{

}

// Called before render is available
bool ModuleGoManager::Init()
{
	bool ret = true;

	CreateRootGameObject();
		
	return ret;
}

bool ModuleGoManager::Start()
{

	return true;
}

// Called every draw update
update_status ModuleGoManager::PreUpdate(float dt)
{
	update_status ret = UPDATE_CONTINUE;

	return ret;
}

update_status ModuleGoManager::Update(float dt)
{
	if (App->input->file_was_dropped)
	{
		char droppedFile[1024];
		strcpy(droppedFile, App->input->dropped_file);
		std::string onlyName = App->importer->FileName(droppedFile);
		LoadGO(onlyName.data());
	}

	std::vector<GameObject*>::iterator it = root->childs.begin();
	while (it != root->childs.end())
	{
		(*it)->Update();
		it++;
	}

	return UPDATE_CONTINUE;
}

update_status ModuleGoManager::PostUpdate(float dt)
{
	
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleGoManager::CleanUp()
{
	if (root)
	{
		delete root;
	}

	return true;
}

std::vector<GameObject*> ModuleGoManager::LoadGO(const char* fileName)
{
	GameObject* sceneRoot = App->importer->LoadVMesh(fileName);
	std::vector<GameObject*> ret;
	if (sceneRoot && sceneRoot->childs.empty() == false)
	{
		for (std::vector<GameObject*>::iterator childs = sceneRoot->childs.begin(); childs != sceneRoot->childs.end(); childs++)
		{
			(*childs)->parent = root;
			root->childs.push_back((*childs));
			ret.push_back((*childs));
		}
		sceneRoot->childs.clear();
		delete sceneRoot;
		LOG("Loaded %s", fileName);
	}
	else
	{
		LOG("Failed to load %s", fileName);
	}
	return ret;
}

bool ModuleGoManager::DeleteGameObject(GameObject* toErase)
{
	if (toErase)
	{
		delete toErase;
		return true;
	}
	return false;
	
}

void ModuleGoManager::CleanName(char* toClean)
{
	char* searcher = toClean;
	int n = 0;
	while (*searcher != '\0')
	{
		if (*searcher == '_')
		{
			*searcher = ' ';
		}
		if (*searcher == '$' || n == NAME_MAX_LEN)
		{
			*searcher = '\0';
			break;
		}
		n++;
		searcher++;
	}
}

void ModuleGoManager::CreateRootGameObject()
{
	GameObject* ret = new GameObject();

	//Setting Name
	strcpy(ret->name, "Root");

	//Setting parent
	ret->parent = nullptr;

	//Setting transform
	math::Quat rot = math::Quat::identity;

	root = ret;
}