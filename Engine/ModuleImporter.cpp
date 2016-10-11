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

	uint bytes = 0;

	//					rot + scal + pos				nMeshes
	uint file_0Size =	sizeof(float) * (4 + 3 + 3) +	sizeof(uint) * 1;
	char* file_0 = new char[file_0Size];
	char* file_0It = file_0;

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

	bytes = sizeof(float) * 10;
	memcpy(file_0It, transform, bytes);
	file_0It += bytes;

	const uint nMeshes =  NodetoLoad->mNumMeshes;

	bytes = sizeof(uint);
	memcpy(file_0It, &nMeshes, bytes);
	file_0It += bytes;

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

		std::string tmp(textureName);
		uint textureNameLen = tmp.length() + 1;

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

							
			uint meshSize =
				//Mesh size
				sizeof(uint) +

				//num_vertices							vertices								normals
				sizeof(uint) +							sizeof(float) * num_vertices * 3 +		sizeof(float) * num_vertices * 3

				//texture Coords						texture name length						tetxture name
				+ sizeof(float) * num_vertices * 2 +	sizeof(uint) +							sizeof(char) * textureNameLen

				//colors								num indices								indices
				+ sizeof(float) * 3 +					sizeof(uint) +							sizeof(uint) * num_indices;

			meshes[n] = new char[meshSize];
			char* meshIt = meshes[n];

			//Mesh size
			bytes = sizeof(uint);
			memcpy(meshIt, &meshSize, bytes);
			meshIt += bytes;

			//Num vertices
			bytes = sizeof(uint);
			memcpy(meshIt, &num_vertices, bytes);
			meshIt += bytes;

			//Vertices
			bytes = sizeof(float) * num_vertices * 3;
			memcpy(meshIt, vertices, bytes);
			meshIt += bytes;

			//Normals
			bytes = sizeof(float) * num_vertices * 3;
			memcpy(meshIt, normals, bytes);
			meshIt += bytes;

			//texture coords
			bytes = sizeof(float) * num_vertices * 2;
			memcpy(meshIt, textureCoords, bytes);
			meshIt += bytes;

			//Texture name len
			bytes = sizeof(uint);
			memcpy(meshIt, &textureNameLen, bytes);
			meshIt += bytes;

			//Texture name
			bytes = sizeof(char) * textureNameLen;
			memcpy(meshIt, textureName, bytes);
			meshIt += bytes;

			//Color
			bytes = sizeof(float) * 3;
			memcpy(meshIt, color, bytes);
			meshIt += bytes;

			//num_indices
			bytes = sizeof(uint);
			memcpy(meshIt, &num_indices, bytes);
			meshIt += bytes;

			//indices
			bytes = sizeof(uint) * num_indices;
			memcpy(meshIt, indices, bytes);
			meshIt += bytes;
		}
	}

	uint nChilds = NodetoLoad->mNumChildren;
	uint* childsSize = new uint[nChilds];
	std::vector<std::string> childs;


	uint childFileSize =
		//nChilds			each child size
		sizeof(uint) +		sizeof(uint) * nChilds;


	//Loading child nodes
	for (int n = 0; n < nChilds; n++)
	{
		std::string toPush(NodetoLoad->mChildren[n]->mName.data);
		childs.push_back(toPush);
		childsSize[n] = toPush.length() + 1;
		childFileSize += sizeof(char) * childsSize[n];
	}

	char* file_childs = new char[childFileSize];
	char* childsIt = file_childs;

	//nCHilds
	bytes = sizeof(uint);
	memcpy(childsIt, &nChilds, bytes);
	childsIt += bytes;

	//size of each child
	bytes = sizeof(uint) * nChilds;
	memcpy(childsIt, childsSize, bytes);
	childsIt += bytes;

	for (int n = 0; n < nChilds; n++)
	{
		//a child
		bytes = sizeof(char) * childsSize[n];
		memcpy(childsIt, childs[n].data(), bytes);
		childsIt += bytes;
	}

	//TMP FOR LOAD AND CHECKING
	char* toLoad = file_0;
	file_0It = toLoad;

	float _transform[10];
	bytes = sizeof(float) * 10;
	memcpy(_transform, toLoad, bytes);
	file_0It += bytes;

	uint _nMeshes = 0;
	bytes = sizeof(uint);
	memcpy(&_nMeshes, file_0It, bytes);
	file_0It += bytes;

	//END FOR TMP LOAD


	//Importing also all the childs
	for (int n = 0; n < nChilds; n++)
	{
		ImportGameObject(path, NodetoLoad->mChildren[n], scene);
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