#include "Globals.h"
#include "Application.h"

#include "ModuleImporter.h"

#include "ModuleFileSystem.h"

#include "GameObject.h"
#include "AllComponents.h"

#include "ModuleResourceManager.h"

#include "AllResources.h"

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
	ilutRenderer(ILUT_OPENGL);

	LOG("Importing assets");
	ImportFromFolder("Assets");

	return true;
}

// Called before quitting
bool ModuleImporter::CleanUp()
{
	aiDetachAllLogStreams();

	return true;
}




// ------------------------------- IMPORTING ------------------------------- 


void ModuleImporter::ImportFromFolder(const char * path)
{
	LOG("Importing all assets in folder: %s", path);
	std::vector<std::string> folders;
	std::vector<std::string> files;
	App->fs->GetFilesIn(path, &folders, &files);

	for (uint n = 0; n < files.size(); n++)
	{
		std::string toSend(path);
		toSend += "/";
		toSend += files[n].data();
		Import(toSend.data());
	}
	files.clear();
	for (uint n = 0; n < folders.size(); n++)
	{
		std::string toSend(path);
		toSend += "/";
		toSend += folders[n].data();
		ImportFromFolder(toSend.data());
	}

}

void ModuleImporter::Import(const char * path)
{
	if (Import3dScene(path) == false)
	{
		ImportImage(path);
	}
}

bool ModuleImporter::Import3dScene(const char * filePath)
{
	//Making sure the file recieved is supported by the assimp library
	std::string fmt = FileFormat(filePath);
	std::string supportedFormats;
	supportedFormats += " FBX";
	for (uint n = 0; n < aiGetImportFormatCount(); n++)
	{
		supportedFormats += " ";
		supportedFormats += aiGetImportFormatDescription(n)->mFileExtensions;		
	}
	if (supportedFormats.find(fmt) == std::string::npos)
	{
		return false;
	}

	LOG("\n ------ [Started importing 3D Scene] ------ ");
	LOG("Importing 3D scene: %s", filePath);

	//Loading the aiScene from Assimp
	const aiScene* scene = aiImportFileEx(filePath, aiProcessPreset_TargetRealtime_MaxQuality, App->fs->GetAssimpIO());

	if (scene != nullptr)
	{
		if (scene->HasMeshes())
		{
			ImportGameObject(filePath, scene->mRootNode, scene);
		}
		if (scene)
		{
			aiReleaseImport(scene);
			scene = nullptr;
		}
		LOG(" ------ [End of importing 3D Scene] ------\n");
	}
	else
	{
		LOG("Error loading scene %s", filePath);
	}
	return true;
}



bool ModuleImporter::ImportImage(const char * filePath)
{
	// Extracted from
	//http://openil.sourceforge.net/features.php
	std::string supportedFormats("bmp dcx dds hdr icns ico cur iff gif jpg jpe jpeg jp2 lbm png raw tif tga");

	if (supportedFormats.find(FileFormat(filePath)) == std::string::npos)
	{
		return false;
	}

	std::string saveName("Library/Textures/");
	saveName += FileName(filePath);
	saveName += TEXTURE_FORMAT;

	LOG("\nStarted importing texture %s", filePath);
	char* buffer;
	uint size;

	size = App->fs->Load(filePath, &buffer);
	if (size > 0)
	{
		ILuint image;
		ILuint newSize;
		ILubyte *data;

		ilGenImages(1, &image);
		ilBindImage(image);

		if (ilLoadL(IL_TYPE_UNKNOWN, (const void*)buffer, size))
		{
			ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);// To pick a specific DXT compression use
			newSize = ilSaveL(IL_DDS, NULL, 0); // Get the size of the data buffer

			if (newSize > 0)
			{
				data = new ILubyte[size]; // allocate data buffer

				ilEnable(IL_FILE_OVERWRITE);
				if (ilSaveL(IL_DDS, data, newSize) > 0)
				{
					// Save to buffer with the ilSaveIL function
					App->fs->Save(saveName.data(), (const char*)data, newSize);
					LOG("Succesfully imported!");
				}
				else
				{
					LOG("devIl couldn't create the .dds!");
				}
				RELEASE_ARRAY(data);
			}
		}
		else
		{
			LOG("devIl couldn't load the image");
		}
		ilDeleteImages(1, &image);
	}
	else
	{
		LOG("Couldn't open the file!");
	}
	RELEASE_ARRAY(buffer);
	return true;
}



void ModuleImporter::ImportGameObject(const char* path, const aiNode* NodetoLoad, const aiScene* scene, bool isChild, const char* RootName)
{
	//Setting Name

	char name[MAXLEN];
	if (isChild)
	{
		//If it isn't the root node, the file name will be the object's name
		memcpy(name, NodetoLoad->mName.data, NodetoLoad->mName.length + 1);
	}
	else
	{
		//If it's the root node, the file name will be the FBX's name
		strcpy(name, FileName(path).data());
	}

	LOG("Importing GameObject %s", name);

	//					rot + scal + pos				nMeshes			Material
	uint transformSize = sizeof(float) * (4 + 3 + 3) + sizeof(uint) + sizeof(uint);
	char* transform = new char[transformSize];
	char* transformIt = transform;

	aiQuaternion rot;
	aiVector3D scal;
	aiVector3D pos;

	NodetoLoad->mTransformation.Decompose(scal, rot, pos);

	float t[10];
	t[0] = rot.x;
	t[1] = rot.y;
	t[2] = rot.z;
	t[3] = rot.w;
	t[4] = scal.x;
	t[5] = scal.y;
	t[6] = scal.z;
	t[7] = pos.x;
	t[8] = pos.y;
	t[9] = pos.z;

	transformIt = CopyMem<float>(transformIt, t, 10);

	transformIt = CopyMem<uint>(transformIt, &NodetoLoad->mNumMeshes);

	std::vector<uint> materials;
	std::vector<std::string> meshes;
	uint nMeshes = NodetoLoad->mNumMeshes;
	for (uint n = 0; n <nMeshes; n++)
	{
		if (isChild == false)
		{
			RootName = name;
		}

		uint matIndex;

		aiMesh* toLoad = scene->mMeshes[NodetoLoad->mMeshes[n]];

		char meshName[256];
		sprintf(meshName, "%s_%i", name, n);

		meshes.push_back(ImportMesh(toLoad, scene, meshName, RootName, matIndex));
		materials.push_back(matIndex);
	}
	uint hasMaterial = 0;
	if (materials.empty() == false)
	{
		ImportMaterial(scene, materials, name);
		hasMaterial = 1;
	}
	transformIt = CopyMem<uint>(transformIt, &hasMaterial);


	uint nChilds = NodetoLoad->mNumChildren;
	uint* childsSize = new uint[nChilds];
	std::vector<std::string> childs;

	uint childFileSize =
		//nChilds			each child size
		sizeof(uint) + sizeof(uint) * nChilds;


	//Loading child nodes
	for (uint n = 0; n < nChilds; n++)
	{
		std::string toPush(NodetoLoad->mChildren[n]->mName.data);
		childs.push_back(toPush);
		childsSize[n] = toPush.length() + 1;
		childFileSize += sizeof(char) * childsSize[n];
	}

	char* file_childs = new char[childFileSize];
	char* childsIt = file_childs;

	//nCHilds
	childsIt = CopyMem<uint>(childsIt, &nChilds);

	//size of each child
	childsIt = CopyMem<uint>(childsIt, childsSize, nChilds);

	for (uint n = 0; n < nChilds; n++)
	{
		//a child
		childsIt = CopyMem<char>(childsIt, (childs[n].data()), childsSize[n]);
	}

	//Getting the total size of the real file
	uint realFileSize = 0;
	realFileSize += transformSize;
	realFileSize += 256 * nMeshes + 256 * hasMaterial;
	realFileSize += childFileSize;

	//Copying all the buffers we created into a single bigger buffer
	char* realFile = new char[realFileSize];
	char* realIt = realFile;

	//file_0
	realIt = CopyMem<char>(realIt, transform, transformSize);

	for (int n = 0; n < nMeshes; n++)
	{
		realIt = CopyMem<char>(realIt, meshes[n].data(), 256);
	}
	if (hasMaterial)
	{
		realIt = CopyMem<char>(realIt, name, 256);
	}

	//childs
	realIt = CopyMem<char>(realIt, file_childs, childFileSize);

	RELEASE_ARRAY(transform);
	RELEASE_ARRAY(childsSize);
	RELEASE_ARRAY(file_childs);

	// ---------------- Creating the save file and writting it -----------------------------------------

	std::string toCreate("Library/vGOs/");
	if (isChild)
	{
		toCreate += RootName;
		toCreate += "/";
	}
	else
	{
		std::string dir("Library/vGOs/");
		dir += name;
		App->fs->CreateDir(dir.data());
	}

	toCreate += name;
	toCreate += GO_FORMAT;
	App->fs->Save(toCreate.data(), realFile, realFileSize);

	RELEASE_ARRAY(realFile);

	//Importing also all the childs
	for (uint n = 0; n < NodetoLoad->mNumChildren; n++)
	{
		if (isChild)
		{
			ImportGameObject(path, NodetoLoad->mChildren[n], scene, true, RootName);
		}
		else
		{
			ImportGameObject(path, NodetoLoad->mChildren[n], scene, true, name);
		}
	}
}

std::string ModuleImporter::ImportMesh(aiMesh* toLoad, const aiScene* scene, const char* name, const char* dir ,uint& materialID)
{
	//Importing vertex
	uint num_vertices = toLoad->mNumVertices;
	float* vertices = new float[num_vertices * 3];
	memcpy_s(vertices, sizeof(float) * num_vertices * 3, toLoad->mVertices, sizeof(float) * num_vertices * 3);

	AABB aabb;
	aabb.SetNegativeInfinity();

	float* it = vertices;
	for (uint n = 0; n < num_vertices * 3; n += 3)
	{
		float* x = it;
		float* y = x;
		y++;
		float* z = y;
		z++;
		aabb.Enclose(float3(*x, *y, *z));
		it += 3;
	}

	//Importing normals
	float* normals = nullptr;
	uint numNormals = 0;
	if (toLoad->HasNormals())
	{
		numNormals = num_vertices;
		normals = new float[num_vertices * 3];
		memcpy(normals, toLoad->mNormals, sizeof(float) * num_vertices * 3);
	}

	//Importing texture coords
	float* textureCoords = nullptr;
	uint numTextureCoords = 0;
	if (toLoad->HasTextureCoords(0))
	{
		numTextureCoords = num_vertices;
		textureCoords = new float[num_vertices * 2];

		aiVector3D* tmpVect = toLoad->mTextureCoords[0];
		for (uint n = 0; n < num_vertices * 2; n += 2)
		{
			textureCoords[n] = tmpVect->x;
			textureCoords[n + 1] = tmpVect->y;
			tmpVect++;
		}
	}

	//Importing texture path for this mesh
	materialID = toLoad->mMaterialIndex;

	//Importing index (3 per face)
	uint num_indices = 0;
	uint* indices = nullptr;

	aiFace* currentFace = toLoad->mFaces;

	num_indices = toLoad->mNumFaces * 3;
	indices = new uint[num_indices];
	//If this boolean is still false at the end of the for bucle, not a single face had been loaded. This mesh is unexistant
	bool meshExists = false;

	for (uint i = 0; i < num_indices; i += 3)
	{
		if (currentFace->mNumIndices != 3)
		{
			LOG("------------------------------------------\nA loaded face had %i vertices, will be ignored!\n------------------------------------------", currentFace->mNumIndices);
		}
		else
		{
			meshExists = true;
			indices[i] = currentFace->mIndices[0];
			indices[i + 1] = currentFace->mIndices[1];
			indices[i + 2] = currentFace->mIndices[2];
		}
		currentFace++;
	}

	uint meshSize =
		//Mesh size		//Mesh exists?
		sizeof(uint) + sizeof(bool) +

		//num_vertices				   vertices				num_normals   normals
		sizeof(uint) + sizeof(float) * num_vertices * 3 + sizeof(uint) + sizeof(float) * numNormals * 3

		//num_texture coords  texture Coords		             
		+ sizeof(uint) + sizeof(float) * numTextureCoords * 2 +

		//num indices								indices
		+sizeof(uint) + sizeof(uint) * num_indices
		//aabb
		+ sizeof(float) * 6;

	char* mesh = new char[meshSize];
	char* meshIt = mesh;

	//Does this mesh actually exist?
	meshIt = CopyMem<bool>(meshIt, &meshExists);

	//Num vertices
	meshIt = CopyMem<uint>(meshIt, &num_vertices);

	//Vertices
	meshIt = CopyMem<float>(meshIt, vertices, num_vertices * 3);

	//Num Normals
	meshIt = CopyMem<uint>(meshIt, &numNormals);

	if (numNormals > 0)
	{
		//Normals
		meshIt = CopyMem<float>(meshIt, normals, numNormals * 3);
	}

	//Num texture coords
	meshIt = CopyMem<uint>(meshIt, &numTextureCoords);

	if (numTextureCoords > 0)
	{
		//texture coords
		meshIt = CopyMem<float>(meshIt, textureCoords, numTextureCoords * 2);
	}

	//num_indices
	meshIt = CopyMem<uint>(meshIt, &num_indices);

	//indices
	meshIt = CopyMem<uint>(meshIt, indices, num_indices);

	//AABB
	meshIt = CopyMem<float3>(meshIt, &aabb.maxPoint);
	meshIt = CopyMem<float3>(meshIt, &aabb.minPoint);


	RELEASE_ARRAY(vertices);
	RELEASE_ARRAY(normals);
	RELEASE_ARRAY(textureCoords);
	RELEASE_ARRAY(indices);

	std::string toCreate("Library/Meshes/");
//	toCreate += dir;
//	App->fs->CreateDir(toCreate.data());

	toCreate += "/";
	toCreate += name;
	toCreate += MESH_FORMAT;
	App->fs->Save(toCreate.data(), mesh, meshSize);

	RELEASE_ARRAY(mesh);

	return name;
}

std::string ModuleImporter::ImportMaterial(const aiScene * scene, std::vector<uint>& matsIndex, const char* matName)
{
	if (matsIndex.empty() == false)
	{
		uint realSize = sizeof(uint);
		uint nTextures = matsIndex.size();

		uint* materialsSize = new uint[matsIndex.size()];
		char** materials = new char*[matsIndex.size()];
		uint n = 0;
		for (std::vector<uint>::iterator it = matsIndex.begin(); it != matsIndex.end(); it++)
		{
			aiString texturePath;
			scene->mMaterials[(*it)]->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &texturePath);
			char tmp[1024];
			strcpy(tmp, texturePath.data);			

			std::string textureName = FileName(tmp);
			uint textureNameLen = textureName.length() + 1;

			//Importing color for this mesh
			aiColor3D col;
			scene->mMaterials[(*it)]->Get(AI_MATKEY_COLOR_DIFFUSE, col);
			float color[3] = { col.r, col.g, col.b };

			materialsSize[n] = sizeof(uint) + sizeof(char) * textureNameLen + sizeof(float3);
			realSize += materialsSize[n];
			materials[n] = new char[materialsSize[n]];
			char* materialIt = materials[n];

			materialIt = CopyMem<uint>(materialIt, &textureNameLen, 1);
			materialIt = CopyMem<char>(materialIt, textureName.data(), textureNameLen);
			materialIt = CopyMem<float>(materialIt, color, 3);

			n++;
		}

		char* realMat = new char[realSize];
		char* realIt = realMat;
		realIt = CopyMem<uint>(realIt, &nTextures);

		for (int n = 0; n < matsIndex.size(); n++)
		{
			realIt = CopyMem<char>(realIt, materials[n], materialsSize[n]);
		}
		
		std::string toCreate("Library/Materials/");
		toCreate += matName;
		toCreate += MATERIAL_FORMAT;
		App->fs->Save(toCreate.data(), realMat, realSize);	

		for (int n = 0; n < matsIndex.size(); n++)
		{
			RELEASE_ARRAY(materials[n]);
		}
		RELEASE_ARRAY(materialsSize);
		RELEASE_ARRAY(materials);
		RELEASE_ARRAY(realMat);
		return matName;
	}
	return NULL;
}



// ------------------------------- LOADING ------------------------------- 


GameObject * ModuleImporter::LoadVgo(const char * fileName_NoFileType, GameObject* parent, char* meshesFolder)
{
	std::string fileName (fileName_NoFileType);

	char* file = nullptr;
	std::string path("Library/vGOs/");
	if (parent && meshesFolder)
	{
		path += meshesFolder;
		path += "/";
	}
	else
	{
		LOG("\n ------- [Began loading Vgo %s] ---------", fileName.data());
	}

	path += fileName;
	path += GO_FORMAT;

	LOG("Loading vgo %s", path.data());

	if (App->fs->Exists(path.data()))
	{
		int size = App->fs->Load(path.data(), &file);
		if (file != nullptr && size > 0)
		{			
			char* It = file;

			//Creating basic components for a GameObject
			GameObject* ret = new GameObject;
			Transform* trans = (Transform*)ret->AddComponent(Component::Type::C_transform);

			//Setting name
			ret->SetName(fileName.data());
			//Setting parent
			ret->parent = parent;

			//Setting transform
			float _transform[10];
			uint bytes = sizeof(float) * 10;
			memcpy(_transform, file, bytes);
			It += bytes;

			trans->SetLocalRot(_transform[0], _transform[1], _transform[2], _transform[3]);
			trans->SetLocalScale(_transform[4], _transform[5], _transform[6]);
			trans->SetLocalPos(_transform[7], _transform[8], _transform[9]);

			trans->UpdateEditorValues();

			//Number of meshes
			uint nMeshes = 0;
			bytes = sizeof(uint);
			memcpy(&nMeshes, It, bytes);
			It += bytes;

			//HasMaterial
			uint hasMaterial = 0;
			bytes = sizeof(uint);
			memcpy(&hasMaterial, It, bytes);
			It += bytes;

			//Loading each mesh
			for (uint n = 0; n < nMeshes; n++)
			{
				char meshName[256];
				bytes = sizeof(char) * 256;
				memcpy(&meshName, It, bytes);
				It += bytes;
				strcat(meshName, MESH_FORMAT);
				ret->AddComponent(Component::Type::C_mesh, meshName);
				ret->SetOriginalAABB();
			}

			if (hasMaterial != 0)
			{
				char materialName[256];
				bytes = sizeof(char) * 256;
				memcpy(&materialName, It, bytes);
				It += bytes;
				strcat(materialName, MATERIAL_FORMAT);
				ret->AddComponent(Component::Type::C_material, materialName);
			}
			
			//Num childs
			uint nChilds = 0;
			bytes = sizeof(uint);
			memcpy(&nChilds, It, bytes);
			It += bytes;

			if (nChilds > 0)
			{
				//Length of each child string (in chars)
				uint* sizeOfChilds = new uint[nChilds];
				bytes = sizeof(uint) * nChilds;
				memcpy(sizeOfChilds, It, bytes);
				It += bytes;

				std::vector<std::string> childs;
				//Loading each child name into a separate string
				for (uint n = 0; n < nChilds; n++)
				{
					char* name = new char[sizeOfChilds[n]];
					bytes = sizeof(char) * sizeOfChilds[n];
					memcpy(name, It, bytes);
					It += bytes;

					childs.push_back(std::string(name));
					delete[] name;
				}
				delete[] sizeOfChilds;


				if (parent == nullptr)
				{
					meshesFolder = new char[fileName.length() + 1];
					memcpy(meshesFolder, fileName.data(), sizeof(char) * (fileName.length() + 1));
				}

				std::vector<std::string>::iterator childNames = childs.begin();
				while (childNames != childs.end())
				{
					std::string thisChild(*childNames);
					GameObject* child = LoadVgo(thisChild.data(), ret, meshesFolder);
					if (child)
					{
						ret->childs.push_back(child);
					}
					childNames++;
				}
				if (parent == nullptr)
				{
					RELEASE_ARRAY(meshesFolder);
				}
			}

			delete[] file;

			return ret;
		}
		else
		{
			LOG("Something went wrong while loading %s", fileName.data());
		}
	}
	else
	{
		LOG("Woops! This .vgo doesn't really exist.")
	}

	return nullptr;
}

R_mesh* ModuleImporter::LoadMesh(const char * path)
{
	char* file = nullptr;
	std::string filePath("Library/Meshes/");
	R_mesh* newMesh = nullptr;

	filePath += path;

	LOG("Loading mesh %s", filePath.data());

	if (App->fs->Exists(filePath.data()))
	{
		int size = App->fs->Load(filePath.data(), &file);
		if (file != nullptr && size > 0)
		{
			char* It = file;

			//Does this mesh exist?
			bool _meshExists = true;
			uint bytes = sizeof(bool);
			memcpy(&_meshExists, It, bytes);
			It += bytes;

			if (_meshExists == true)
			{
				newMesh = new R_mesh();

				newMesh->file = File(filePath.data());

				//Num vertices
				bytes = sizeof(uint);
				memcpy(&newMesh->num_vertices, It, bytes);
				It += bytes;

				//Actual vertices
				newMesh->vertices = new float3[newMesh->num_vertices];
				bytes = sizeof(float3) * newMesh->num_vertices;
				memcpy(newMesh->vertices, It, bytes);
				It += bytes;

				//Generating vertices buffer
				glGenBuffers(1, (GLuint*) &(newMesh->id_vertices));
				glBindBuffer(GL_ARRAY_BUFFER, newMesh->id_vertices);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * newMesh->num_vertices * 3, newMesh->vertices, GL_STATIC_DRAW);
				//endof Generating vertices buffer


				//Num normals
				bytes = sizeof(uint);
				memcpy(&newMesh->num_normals, It, bytes);
				It += bytes;

				if (newMesh->num_normals > 0)
				{
					//Normals
					newMesh->normals = new float3[newMesh->num_normals];
					bytes = sizeof(float3) * newMesh->num_normals;
					memcpy(newMesh->normals, It, bytes);
					It += bytes;

					//Generating normals buffer
					glGenBuffers(1, (GLuint*) &(newMesh->id_normals));
					glBindBuffer(GL_ARRAY_BUFFER, newMesh->id_normals);
					glBufferData(GL_ARRAY_BUFFER, sizeof(float) * newMesh->num_normals * 3, newMesh->normals, GL_STATIC_DRAW);
					//endOf Generating normals buffer
				}

				//Num texture coords
				bytes = sizeof(uint);
				memcpy(&newMesh->num_textureCoords, It, bytes);
				It += bytes;

				if (newMesh->num_textureCoords > 0)
				{
					//Texture coords
					float* textureCoords = new float[newMesh->num_vertices * 2];
					bytes = sizeof(float) * newMesh->num_normals * 2;
					memcpy(textureCoords, It, bytes);
					It += bytes;

					//Generating UVs buffer
					glGenBuffers(1, (GLuint*) &(newMesh->id_textureCoords));
					glBindBuffer(GL_ARRAY_BUFFER, newMesh->id_textureCoords);
					glBufferData(GL_ARRAY_BUFFER, sizeof(float) * newMesh->num_textureCoords * 2, textureCoords, GL_STATIC_DRAW);
					//endOF Generatinv UVs buffer
					RELEASE_ARRAY(textureCoords);
				}

				//Num indices
				bytes = sizeof(uint);
				memcpy(&newMesh->num_indices, It, bytes);
				It += bytes;

				//Actual indices
				newMesh->indices = new uint[newMesh->num_indices];
				bytes = sizeof(uint) * newMesh->num_indices;
				memcpy(newMesh->indices, It, bytes);
				It += bytes;

				//Generating indices buffer
				glGenBuffers(1, (GLuint*) &(newMesh->id_indices));
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh->id_indices);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * newMesh->num_indices, newMesh->indices, GL_STATIC_DRAW);
				//endOf generating indices buffer


				//AABB maxPoint
				bytes = sizeof(float3);
				memcpy(&newMesh->aabb.maxPoint, It, bytes);
				It += bytes;

				//AABB minPoint
				memcpy(&newMesh->aabb.minPoint, It, bytes);
				It += bytes;
			}
		}
		RELEASE_ARRAY(file);
	}
	return newMesh;
}

R_Material* ModuleImporter::LoadMaterial(const char * path)
{
	char* file = nullptr;
	std::string filePath("Library/Materials/");
	R_Material* mat = nullptr;

	filePath += path;

	LOG("Loading mesh %s", filePath.data());

	if (App->fs->Exists(filePath.data()))
	{
		int size = App->fs->Load(filePath.data(), &file);
		if (file != nullptr && size > 0)
		{
			char* It = file;
			mat = new R_Material();

			uint bytes = 0;
			uint nTextures = 0;

			mat->file = path;

			//NumTextures
			uint numTextures = 0;
			bytes = sizeof(uint);
			memcpy(&numTextures, It, bytes);
			It += bytes;

			for (int n = 0; n < numTextures; n++)
			{
				//Texture name Len
				uint textureNameLen = 0;
				bytes = sizeof(uint);
				memcpy(&textureNameLen, It, bytes);
				It += bytes;

				if (textureNameLen > 1)
				{
					//Texture name
					char* textureName = new char[textureNameLen];
					bytes = sizeof(char) * textureNameLen;
					memcpy(textureName, It, bytes);
					It += bytes;
					std::string path(textureName);
					path += TEXTURE_FORMAT;
					R_Texture* toAdd = (R_Texture*)App->resources->LinkResource(path.data(), Component::C_Texture);
					if (toAdd != nullptr)
					{
						mat->textures.push_back(toAdd);
					}

					delete[] textureName;
				}
				//Color
				float color[3];
				bytes = sizeof(float) * 3;
				memcpy(color, It, bytes);
				It += bytes;
				mat->color[0] = color[0];
				mat->color[1] = color[1];
				mat->color[2] = color[2];
				mat->color[4] = 1.0f;
			}
			RELEASE(file);
		}
	}
	return mat;
}

R_Texture* ModuleImporter::LoadTexture(const char* path)
{
	R_Texture* ret = nullptr;
	if (*path == '\0')
	{
		return ret;
	}

	std::string name = FileName(path);

	std::string fullPath(App->fs->GetWrittingDirectory());
	fullPath += "Library\\Textures\\";
	fullPath += name;
	fullPath += TEXTURE_FORMAT;

	LOG("Loading Texture %s", path);

	char tmp[1024];
	strcpy(tmp, fullPath.data());
	uint ID = ilutGLLoadImage(tmp);

	if (ID != 0)
	{
		ret = new R_Texture();
		ret->file = path;
		ret->bufferID = ID;
		return ret;
	}
	else
	{
		LOG("Error loading texture %s", path);
		for (ILenum error = ilGetError(); error != IL_NO_ERROR; error = ilGetError())
		{
			LOG("devIL got error %d", error);
			//For some reason, this will break and cause a crash
			//LOG("%s", iluErrorString(error));
		}
		return ret;
	}
}



// ------------------------------- UTILITY ------------------------------- 

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
			if (tmp == name)
			{
				return std::string("");
			}
		}
		tmp++;
		return std::string(tmp);
}

std::string ModuleImporter::FileName(const char * file)
{
	char name[1024];
	strcpy(name, file);
	char* start = name;
	int size = 0;
	while (*start != '\0')
	{
		size++;
		start++;
	}

	char*  end = start;
	while (size >= 0  && *start != '/' && *start != '\\')
	{
		size--;
		start--;
	}
	start++;

	while (*end != '.' && end != start)
	{
		end--;
	}
	if (end != start)
	{
		*end = '\0';
	}
	return std::string(start);
}

std::string ModuleImporter::File(const char * file)
{
	const char* start = file;

	while (*start != '\0')
	{
		start++;
	}

	while (*start != '/' && *start != '\\' && start != file)
	{
		start--;
	}
	if (start != file)
	{
		start++;
	}
	return std::string(start);
}
