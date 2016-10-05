#include "Globals.h"
#include "Application.h"

#include "ModuleGOmanager.h"

#include "ModuleFileSystem.h"
#include "ModuleInput.h"

#include "OpenGL.h"

#include "Devil\include\il.h"
#include "Devil\include\ilu.h"
#include "Devil\include\ilut.h"


#include "Assimp\include\cimport.h"
#include "Assimp\include\scene.h"
#include "Assimp\include\postprocess.h"
#include "Assimp\include\cfileio.h"

#pragma comment(lib, "Assimp/libx86/assimp.lib")

#pragma comment(lib, "Devil/libx86/DevIL.lib")
#pragma comment(lib, "Devil/libx86/ILU.lib")
#pragma comment(lib, "Devil/libx86/ILUT.lib")

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

	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);

	ilInit();

	ILuint devilError = ilGetError();

	if (devilError != IL_NO_ERROR)
	{
		printf("Devil Error (ilInit: %s\n", iluErrorString(devilError));
		exit(2);
	}

	CreateRootGameObject();
		
	return ret;
}

bool ModuleGoManager::Start()
{

	//Generating checker texture
	ilutRenderer(ILUT_OPENGL);

	int a = 0;
	GLubyte checkImage[CHECKERS_HEIGHT][CHECKERS_WIDTH][4];
	for (int i = 0; i < CHECKERS_HEIGHT; i++) {
		for (int j = 0; j < CHECKERS_WIDTH; j++) {
			int c = ((((i & 0x8) == 0) ^ (((j & 0x8)) == 0))) * 255;
			checkImage[i][j][0] = (GLubyte)c;
			checkImage[i][j][1] = (GLubyte)c;
			checkImage[i][j][2] = (GLubyte)c;
			checkImage[i][j][3] = (GLubyte)255;
		}
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &id_checkerTexture);
	glBindTexture(GL_TEXTURE_2D, id_checkerTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHECKERS_WIDTH, CHECKERS_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
	glBindTexture(GL_TEXTURE_2D, 0);

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
		C_String format = App->input->DroppedFileFormat();
		if ( format == ".fbx" || format == ".FBX")
		{
			LoadFBX(App->input->dropped_file);
		}
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
	aiDetachAllLogStreams();

	if (root)
	{
		delete root;
	}

	return true;
}

GameObject* ModuleGoManager::LoadFBX(char* path)
{
//	SDL_RWops* file = App->fs->Load(path);

//	aiFileIO aiFile;
//	aiFile.OpenProc(&aiFile, "App->fs->Load(path)", "this");

	GameObject* ret = NULL;

	const aiScene* scene = aiImportFile(path, aiProcessPreset_TargetRealtime_MaxQuality);
	if (scene != nullptr)
	{
		if (scene->HasMeshes())
		{
			ret = LoadGameObject(scene->mRootNode, scene, root);
			root->childs.push_back(ret);
		}
		if (scene)
		{
			aiReleaseImport(scene);
			scene = NULL;
		}
	}
	else
	{
		LOG("Error loading scene %s", path);
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

GameObject* ModuleGoManager::LoadGameObject(const aiNode* toLoad, const aiScene* scene, GameObject* parent)
{
	GameObject* ret = new GameObject();

	//Setting Name
	char tmpName[MAXLEN];
	memcpy(tmpName, toLoad->mName.data, toLoad->mName.length + 1);

	CleanName(tmpName);

	strcpy(ret->name, tmpName);

	//Setting parent
	ret->parent = parent;

	Transform* trans = (Transform*)ret->AddComponent(C_transform);
	//Setting transform
	aiQuaternion rot;
	aiVector3D scal;
	aiVector3D pos;

	toLoad->mTransformation.Decompose(scal, rot, pos);

	trans->SetPos(pos.x, pos.y, pos.z);
	trans->SetScale(scal.x, scal.y, scal.z);
	trans->SetRot(rot.x, rot.y, rot.z, rot.w);

	Material* mat = (Material*)ret->AddComponent(C_material);

	//Loading meshes
	for (int n = 0; n < toLoad->mNumMeshes; n++)
	{   
		mesh* addedMesh = (mesh*)ret->AddComponent(C_mesh);
		addedMesh->LoadMesh(scene->mMeshes[toLoad->mMeshes[n]], scene);
	}

	//Loading child nodes
	for (int n = 0; n < toLoad->mNumChildren; n++)
	{
		ret->childs.push_back(LoadGameObject(toLoad->mChildren[n], scene, ret));
	}

	return ret;
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