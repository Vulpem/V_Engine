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
		(*it)->Draw();
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

uint ModuleGoManager::LoadTexture(char* path)
{
	if (*path == '\0')
	{
		return 0;
	}

	char image[256] = "FBX/";
	strcat(image, path);

	//C_String image = "FBX/";
	//image += path;

	uint ret = ilutGLLoadImage(image);
	

	if (ret != 0)
	{
		id_textures.push_back(ret);
		return ret;
	}
	else
	{
		LOG("Error loading texture %s", path);
		for(ILenum error = ilGetError(); error != IL_NO_ERROR; error = ilGetError())
		{
			LOG("devIL got error %d", error);
		//	LOG("%s", iluErrorString(error));
		}
		return 0;
	}
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

	//Setting transform
	aiQuaternion rot;
	aiVector3D scal;
	aiVector3D pos;

	toLoad->mTransformation.Decompose(scal, rot, pos);

	ret->scale.x = scal.x;
	ret->scale.y = scal.y;
	ret->scale.z = scal.z;

	ret->position.x = pos.x;
	ret->position.y = pos.y;
	ret->position.z = pos.z;

	ret->rotation.x = rot.x;
	ret->rotation.y = rot.y;
	ret->rotation.z = rot.z;
	ret->rotation.w = rot.w;


	//Loading meshes
	for (int n = 0; n < toLoad->mNumMeshes; n++)
	{   
		mesh* tmp = LoadMesh(scene->mMeshes[toLoad->mMeshes[n]], scene);
		ret->meshes.push_back(tmp);
	}

	//Loading child nodes
	for (int n = 0; n < toLoad->mNumChildren; n++)
	{
		ret->childs.push_back(LoadGameObject(toLoad->mChildren[n], scene, ret));
	}

	return ret;
}

mesh* ModuleGoManager::LoadMesh(const aiMesh* toLoad, const aiScene* scene)
{
	mesh* toPush = new mesh;

	char tmpName[MAXLEN];
	memcpy(tmpName, toLoad->mName.data, toLoad->mName.length + 1);
	CleanName(tmpName);
	strcpy(toPush->name, tmpName);

	glGenBuffers(1, (GLuint*) &(toPush->id_vertices));
	glGenBuffers(1, (GLuint*) &(toPush->id_indices));
	
	//Importing vertex
	toPush->num_vertices = toLoad->mNumVertices;
	toPush->vertices = new float[toPush->num_vertices * 3];
	memcpy_s(toPush->vertices, sizeof(float) * toPush->num_vertices * 3, toLoad->mVertices, sizeof(float) * toPush->num_vertices * 3);

	glBindBuffer(GL_ARRAY_BUFFER, toPush->id_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * toPush->num_vertices * 3, toPush->vertices, GL_STATIC_DRAW);

	//Importing normals
	if (toLoad->HasNormals())
	{
		glGenBuffers(1, (GLuint*) &(toPush->id_normals));
		toPush->num_normals = toPush->num_vertices;
		toPush->normals = new float[toPush->num_normals * 3];
		memcpy_s(toPush->normals, sizeof(float) * toPush->num_normals * 3, toLoad->mNormals, sizeof(float) * toPush->num_normals * 3);

		glBindBuffer(GL_ARRAY_BUFFER, toPush->id_normals);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * toPush->num_normals * 3, toPush->normals, GL_STATIC_DRAW);
	}

	//Importing texture coords
	if (toLoad->HasTextureCoords(0))
	{
		glGenBuffers(1, (GLuint*) &(toPush->id_textureCoords));
		toPush->num_textureCoords = toPush->num_vertices;

		toPush->textureCoords = new float[toPush->num_textureCoords * 2];
		
		aiVector3D* tmp = toLoad->mTextureCoords[0];
		for (int n = 0; n < toPush->num_textureCoords * 2; n+=2)
		{
			toPush->textureCoords[n] = tmp->x;
			toPush->textureCoords[n+1] = tmp->y;
			tmp++;
		}

		glBindBuffer(GL_ARRAY_BUFFER, toPush->id_textureCoords);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * toPush->num_textureCoords * 2, toPush->textureCoords, GL_STATIC_DRAW);
	}
	aiString path;
	scene->mMaterials[toLoad->mMaterialIndex]->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &path);

	toPush->texture = App->GO->LoadTexture(path.data);

	//Importing index (3 per face)
	if (toLoad->HasFaces())
	{
		aiFace* currentFace = toLoad->mFaces;

		toPush->num_indices = toLoad->mNumFaces * 3;
		toPush->indices = new uint[toPush->num_indices];
		for (int i = 0; i < toPush->num_indices; i += 3)
		{
			if (currentFace->mNumIndices != 3)
			{
				LOG("A loaded face had more than 3 vertices");// in %s", path);
			}
			else
			{
				toPush->indices[i] = currentFace->mIndices[0];
				toPush->indices[i + 1] = currentFace->mIndices[1];
				toPush->indices[i + 2] = currentFace->mIndices[2];
			}
			currentFace++;
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, toPush->id_indices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * toPush->num_indices, toPush->indices, GL_STATIC_DRAW);
	}

	return toPush;

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

	ret->scale.x = 1;
	ret->scale.y = 1;
	ret->scale.z = 1;

	ret->position.x = 0;
	ret->position.y = 0;
	ret->position.z = 0;

	ret->rotation.x = rot.x;
	ret->rotation.y = rot.y;
	ret->rotation.z = rot.z;
	ret->rotation.w = rot.w;

	root = ret;
}