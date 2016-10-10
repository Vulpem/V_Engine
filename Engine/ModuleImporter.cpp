#include "Globals.h"
#include "Application.h"

#include "ModuleImporter.h"

#include "ModuleFileSystem.h"

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

ModuleImporter::ModuleImporter(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name.create("ModuleImporter");
}

// DestructorF
ModuleImporter::~ModuleImporter()
{

}

// Called before render is available
bool ModuleImporter::Init()
{
	bool ret = true;

	//Init for assimp
	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);

	//Init for devIL
	ilInit();

	ILuint devilError = ilGetError();

	if (devilError != IL_NO_ERROR)
	{
		printf("Devil Error (ilInit: %s\n", iluErrorString(devilError));
		exit(2);
	}
		
	return ret;
}

bool ModuleImporter::Start()
{
	ImportFromFolder("Assets");

	return true;
}

// Called every draw update
update_status ModuleImporter::PreUpdate(float dt)
{
	update_status ret = UPDATE_CONTINUE;

	return ret;
}

update_status ModuleImporter::Update(float dt)
{

	return UPDATE_CONTINUE;
}

update_status ModuleImporter::PostUpdate(float dt)
{
	
	return UPDATE_CONTINUE;
}

void ModuleImporter::ImportFromFolder(const char * path)
{
	std::vector<std::string> folders;
	std::vector<std::string> files;
	App->fs->GetFilesIn(path, &folders, &files);

	for (int n = 0; n < files.size(); n++)
	{
		std::string toSend(path);
		toSend += "/";
		toSend += files[n].data();
		Import3dScene(toSend.data());
	}
	files.clear();
	for (int n = 0; n < folders.size(); n++)
	{
		std::string toSend(path);
		toSend += "/";
		toSend += folders[n].data();
		ImportFromFolder(toSend.data());
	}

}

void ModuleImporter::Import3dScene(const char * filePath)
{
	if (FileFormat(filePath).compare(".fbx"))
	{
		return;
	}

	LOG("Importing mesh: %s", filePath);

	const aiScene* scene = aiImportFileEx(filePath, aiProcessPreset_TargetRealtime_MaxQuality, App->fs->GetAssimpIO());

	if (scene != NULL)
	{
		if (scene->HasMeshes())
		{
			ImportGameObject(filePath, scene->mRootNode, scene);
		}
		if (scene)
		{
			aiReleaseImport(scene);
			scene = NULL;
		}
	}
	else
	{
		LOG("Error loading scene %s", filePath);
	}
}

void ModuleImporter::ImportGameObject(const char* path, const aiNode* NodetoLoad, const aiScene* scene)
{
	//Setting Name
	char name[MAXLEN];
	memcpy(name, NodetoLoad->mName.data, NodetoLoad->mName.length + 1);
	CleanName(name);

	std::string strName(name);
	uint nameLen = strName.length() + 1;

	//					nameLen			name						rot + scal + pos				nMeshes
	uint file_0Size =	sizeof(uint) +	sizeof(char) * nameLen +	sizeof(float) * (4 + 3 + 3) +	sizeof(uint) * 1;
	char* file_0 = new char[file_0Size];
	char* file_0It = file_0;

	memcpy(file_0It, &nameLen, sizeof(uint));
	file_0It += sizeof(uint);

	memcpy(file_0It, name, sizeof(char) * nameLen);
	file_0It+= sizeof(char) * nameLen;

	aiQuaternion rot;
	aiVector3D scal;
	aiVector3D pos;

	NodetoLoad->mTransformation.Decompose(scal, rot, pos);

	float transform[10];
	transform[0] = rot.x;
	transform[1] = rot.y;
	transform[2] = rot.z;
	transform[3] = rot.w;
	transform[4] = scal.x;
	transform[5] = scal.y;
	transform[6] = scal.z;
	transform[7] = pos.x;
	transform[8] = pos.y;
	transform[9] = pos.z;

	memcpy(file_0It, transform, sizeof(float) * 10);
	file_0It += sizeof(float) * 10;

	const uint nMeshes = NodetoLoad->mNumMeshes;

	memcpy(file_0It, &nMeshes, sizeof(uint));

	//TMP FOR LOAD AND CHECKING
	char* toLoad = file_0;

	uint _nameLen = 0;
	memcpy(&_nameLen, toLoad, sizeof(uint));
	toLoad += sizeof(uint);

	char _name[MAXLEN];
	memcpy(_name, toLoad, sizeof(char) * _nameLen);
	file_0It+= sizeof(char) * nameLen;

	float _transform[10];
	memcpy(_transform, toLoad, sizeof(float) * 10);
	file_0It += sizeof(float) * 10;

	uint _nMeshes = 0;
	memcpy(&_nMeshes, tolower, sizeof(uint));

	//END FOR TMP LOAD

	char** meshes = new char*[nMeshes];

	for (int n = 0; n < nMeshes; n++)
	{
		aiMesh* toLoad = scene->mMeshes[NodetoLoad->mMeshes[n]];

		//Importing vertex
		uint num_vertices = toLoad->mNumVertices;
		float* vertices = new float[num_vertices * 3];
		memcpy_s(vertices, sizeof(float) * num_vertices * 3, toLoad->mVertices, sizeof(float) * num_vertices * 3);

		float* normals = NULL;
		//Importing normals
		//Num normals = num_vertices
		if (toLoad->HasNormals())
		{
			normals = new float[num_vertices * 3];
			memcpy_s(normals, sizeof(float) * num_vertices * 3, toLoad->mNormals, sizeof(float) * num_vertices * 3);
		}

		float* textureCoords = NULL;
		//Importing texture coords
		//Num texture Coords = num_vertices
		if (toLoad->HasTextureCoords(0))
		{
			textureCoords = new float[num_vertices * 2];

			aiVector3D* tmp = toLoad->mTextureCoords[0];
			for (int n = 0; n < num_vertices * 2; n += 2)
			{
				textureCoords[n] = tmp->x;
				textureCoords[n + 1] = tmp->y;
				tmp++;
			}
		}

		//Importing texture path for this mesh
		aiString texturePath;
		scene->mMaterials[toLoad->mMaterialIndex]->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &texturePath);
		char textureName[1024];
		strcpy(textureName, texturePath.data);
		CleanName(textureName);

		//Importing color for this mesh
		aiColor3D col;
		scene->mMaterials[toLoad->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, col);
		float color[3] = { col.r, col.g, col.b };

		//Importing index (3 per face)
		uint num_indices = 0;
		uint* indices = NULL;
		if (toLoad->HasFaces())
		{
			aiFace* currentFace = toLoad->mFaces;

			num_indices = toLoad->mNumFaces * 3;
			indices = new uint[num_indices];
			for (int i = 0; i < num_indices; i += 3)
			{
				if (currentFace->mNumIndices != 3)
				{
					LOG("A loaded face had more than 3 vertices from %s", path);
				}
				else
				{
					indices[i] = currentFace->mIndices[0];
					indices[i + 1] = currentFace->mIndices[1];
					indices[i + 2] = currentFace->mIndices[2];
				}
				currentFace++;
			}
		}

		uint nChilds = NodetoLoad->mNumChildren;
		std::vector<std::string> childs;
		//Loading child nodes
		for (int n = 0; n < nChilds; n++)
		{
			std::string toPush(NodetoLoad->mChildren[n]->mName.data);
			childs.push_back(toPush);
		}
	}
}


void ModuleImporter::CleanName(char* toClean)
{
	char* searcher = toClean;
	int n = 0;
	while (*searcher != '\0')
	{
		if (*searcher == '_')
		{
			*searcher = ' ';
		}
		if (*searcher == '$' || n == MAXLEN)
		{
			*searcher = '\0';
			break;
		}
		n++;
		searcher++;
	}
}

std::string ModuleImporter::FileFormat(const char * file)
{
	char name[1024];
	strcpy(name, file);
		char* tmp = name;
		while (*tmp != '\0')
		{
			tmp++;
		}
		while (*tmp != '.')
		{
			tmp--;
		}
		return std::string(tmp);
}

// Called before quitting
bool ModuleImporter::CleanUp()
{
	aiDetachAllLogStreams();

	return true;
}