#include "Globals.h"
#include "Application.h"

#include "ModuleImporter.h"

#include "ModuleFileSystem.h"

#include "GameObject.h"
#include "AllComponents.h"

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
		if (Import3dScene(toSend.data()) == false)
		{
			ImportImage(toSend.data());
		}

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
	saveName += ".dds";

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
					LOG("devIl couldn't create de .dds!");
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

GameObject * ModuleImporter::LoadVgo(const char * fileName_NoFileType, GameObject* parent, char* meshesFolder)
{
	std::string fileName = FileName(fileName_NoFileType);

	char* file = nullptr;
	std::string path("Library/Meshes/");
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
	path += ".vgo";

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
			Material* mat = (Material*)ret->AddComponent(Component::Type::C_material);

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
			uint _nMeshes = 0;
			bytes = sizeof(uint);
			memcpy(&_nMeshes, It, bytes);
			It += bytes;

			char* startOfMesh;

			//Loading each mesh
			for (uint n = 0; n < _nMeshes; n++)
			{
				startOfMesh = It;

				//Total mesh size (just in case)
				uint _meshSize = 0;
				bytes = sizeof(uint);
				memcpy(&_meshSize, It, bytes);
				It += bytes;

				bool _meshExists = true;
				bytes = sizeof(bool);
				memcpy(&_meshExists, It, bytes);
				It += bytes;

				if (_meshExists == false)
				{
					It = startOfMesh;
					It += _meshSize;
				}
				else
				{
					mesh* newMesh = (mesh*)ret->AddComponent(Component::Type::C_mesh);

					//Num vertices
					bytes = sizeof(uint);
					memcpy(&newMesh->num_vertices, It, bytes);
					It += bytes;

					//Actual vertices
					float* vertices = new float[newMesh->num_vertices * 3];
					bytes = sizeof(float) * newMesh->num_vertices * 3;
					memcpy(vertices, It, bytes);
					It += bytes;

					//Generating vertices buffer
					glGenBuffers(1, (GLuint*) &(newMesh->id_vertices));
					glBindBuffer(GL_ARRAY_BUFFER, newMesh->id_vertices);
					glBufferData(GL_ARRAY_BUFFER, sizeof(float) * newMesh->num_vertices * 3, vertices, GL_STATIC_DRAW);
					//endof Generating vertices buffer
					RELEASE_ARRAY(vertices);


					//Num normals
					bytes = sizeof(uint);
					memcpy(&newMesh->num_normals, It, bytes);
					It += bytes;

					if (newMesh->num_normals > 0)
					{
						//Normals
						float* normals = new float[newMesh->num_vertices * 3];
						bytes = sizeof(float) * newMesh->num_normals * 3;
						memcpy(normals, It, bytes);
						It += bytes;

						//Generating normals buffer
						glGenBuffers(1, (GLuint*) &(newMesh->id_normals));
						glBindBuffer(GL_ARRAY_BUFFER, newMesh->id_normals);
						glBufferData(GL_ARRAY_BUFFER, sizeof(float) * newMesh->num_normals * 3, normals, GL_STATIC_DRAW);
						//endOf Generating normals buffer
						RELEASE_ARRAY(normals);
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

					//Texture name Len
					uint textureNameLen = 0;
					bytes = sizeof(uint);
					memcpy(&textureNameLen, It, bytes);
					It += bytes;

					//Texture name
					char* textureName = new char[textureNameLen];
					bytes = sizeof(char) * textureNameLen;
					memcpy(textureName, It, bytes);
					It += bytes;

					if (textureNameLen > 1)
					{
						newMesh->texMaterialIndex = LoadTexture(textureName, mat);
					}
					delete[] textureName;


					//Color
					float color[3];
					bytes = sizeof(float) * 3;
					memcpy(color, It, bytes);
					It += bytes;
					mat->SetColor(color[0], color[1], color[2]);

					//Num indices
					bytes = sizeof(uint);
					memcpy(&newMesh->num_indices, It, bytes);
					It += bytes;

					//Actual indices
					uint* indices = new uint[newMesh->num_indices];
					bytes = sizeof(uint) * newMesh->num_indices;
					memcpy(indices, It, bytes);
					It += bytes;

					//Generating indices buffer
					glGenBuffers(1, (GLuint*) &(newMesh->id_indices));
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh->id_indices);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * newMesh->num_indices, indices, GL_STATIC_DRAW);
					//endOf generating indices buffer

					RELEASE_ARRAY(indices);
				}
			}

			//AABB box
			bytes = sizeof(float) * 3;
			float3 minPoint;
			memcpy(minPoint.ptr(), It, bytes);
			It += bytes;

			float3 maxPoint;
			memcpy(maxPoint.ptr(), It, bytes);
			It += bytes;

			ret->SetOriginalAABB(minPoint, maxPoint);
			
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

int ModuleImporter::LoadTexture(char* path, Material* mat)
{
	if (*path == '\0')
	{
		return -1;
	}

	std::string name = FileName(path);

	std::string fullPath(App->fs->GetWrittingDirectory());
	fullPath += "Library\\Textures\\";
	fullPath += name;
	fullPath += ".dds";

	LOG("Loading Texture %s", path);

	//Checking if the texture is already loaded
	std::vector<std::string>::iterator it = mat->texturePaths.begin();
	int n = 0;
	while (it != mat->texturePaths.end())
	{
		if (name == it->data())
		{
			LOG("It already exists! Passing id %i", mat->textures.at(n));
			return n;
			//return mat->textures.at(n);
		}
		it++;
		n++;
	}

	

	char tmp[1024];
	strcpy(tmp, fullPath.data());
	uint ID = ilutGLLoadImage(tmp);

	if (ID != 0)
	{
		int ret = mat->textures.size();
		mat->textures.push_back(ID);
		mat->texturePaths.push_back(name.data());
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
		return -1;
	}
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

		//					rot + scal + pos				nMeshes
		uint file_0Size = sizeof(float) * (4 + 3 + 3) + sizeof(uint) * 1;
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

		file_0It = CopyMem<float>(file_0It, transform, 10);

		const uint nMeshes = NodetoLoad->mNumMeshes;

		file_0It = CopyMem<uint>(file_0It, &nMeshes);

		//The AABB box that will englobe all the vertices of the meshes
		AABB aabb;
		aabb.SetNegativeInfinity();

		char** meshes = new char*[nMeshes];
		uint* meshSize = new uint[nMeshes];

		for (uint n = 0; n < nMeshes; n++)
		{
			aiMesh* toLoad = scene->mMeshes[NodetoLoad->mMeshes[n]];

			//Importing vertex
			uint num_vertices = toLoad->mNumVertices;
			float* vertices = new float[num_vertices * 3];
			memcpy_s(vertices, sizeof(float) * num_vertices * 3, toLoad->mVertices, sizeof(float) * num_vertices * 3);

			float* it = vertices;
			for (uint n = 0; n < num_vertices * 3; n += 3)
			{
				float* x = it;
				float* y = x;
				y++;
				float* z = y;
				z++;
				aabb.Enclose(float3(*x,*y,*z));
				it += 3;
			}

			//Importing normals
			float* normals = nullptr;
			uint numNormals = 0;
			if (toLoad->HasNormals())
			{
				numNormals = num_vertices;
				normals = new float[num_vertices * 3];
				memcpy_s(normals, sizeof(float) * num_vertices * 3, toLoad->mNormals, sizeof(float) * num_vertices * 3);
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
			aiString texturePath;
			scene->mMaterials[toLoad->mMaterialIndex]->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &texturePath);
			char textureName[1024];
			strcpy(textureName, texturePath.data);

			std::string tmp(textureName);
			uint textureNameLen = tmp.length() + 1;

			//Importing color for this mesh
			aiColor3D col;
			scene->mMaterials[toLoad->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, col);
			float color[3] = { col.r, col.g, col.b };

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
					LOG("------------------------------------------\nA loaded face had %i vertices, will be ignored!\nFrom %s\n%s\n------------------------------------------", currentFace->mNumIndices, name, path);
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

			meshSize[n] =
				//Mesh size		//Mesh exists?
				sizeof(uint) + sizeof(bool) +

				//num_vertices				   vertices				num_normals   normals
				sizeof(uint) + sizeof(float) * num_vertices * 3 + sizeof(uint) + sizeof(float) * numNormals * 3

				//num_texture coords  texture Coords		        texture name length		      tetxture name
				+ sizeof(uint) + sizeof(float) * numTextureCoords * 2 + sizeof(uint) + sizeof(char) * textureNameLen

				//colors								num indices								indices
				+ sizeof(float) * 3 + sizeof(uint) + sizeof(uint) * num_indices;



			meshes[n] = new char[meshSize[n]];
			char* meshIt = meshes[n];

			//Mesh size
			meshIt = CopyMem<uint>(meshIt, meshSize);

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

			//Texture name len
			meshIt = CopyMem<uint>(meshIt, &textureNameLen);

			//Texture name
			meshIt = CopyMem<char>(meshIt, textureName, textureNameLen);

			//Color
			meshIt = CopyMem<float>(meshIt, color, 3);

			//num_indices
			meshIt = CopyMem<uint>(meshIt, &num_indices);

			//indices
			meshIt = CopyMem<uint>(meshIt, indices, num_indices);

			RELEASE_ARRAY(vertices);
			RELEASE_ARRAY(normals);
			RELEASE_ARRAY(textureCoords);
			RELEASE_ARRAY(indices);
		}

		uint AABBFileSize = sizeof(float) * 6;

		char* aabbFile = new char[AABBFileSize];
		char* aabbFile_it = aabbFile;
			//minPoint
		aabbFile_it = CopyMem<float>(aabbFile_it, aabb.minPoint.ptr(), 3);

			//maxPoint
		aabbFile_it = CopyMem<float>(aabbFile_it, aabb.maxPoint.ptr(), 3);			

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
		realFileSize += file_0Size;
		for (uint n = 0; n < nMeshes; n++)
		{
			realFileSize += meshSize[n];
		}
		realFileSize += AABBFileSize;
		realFileSize += childFileSize;

		//Copying all the buffers we created into a single bigger buffer
		char* realFile = new char[realFileSize];
		char* realIt = realFile;

		//file_0
		realIt = CopyMem<char>(realIt, file_0, file_0Size);

		for (uint n = 0; n < nMeshes; n++)
		{
			//each mesh
			realIt = CopyMem<char>(realIt, meshes[n], meshSize[n]);
		}

		//AABB
		realIt = CopyMem<char>(realIt, aabbFile, AABBFileSize);

		//childs
		realIt = CopyMem<char>(realIt, file_childs, childFileSize);

		RELEASE_ARRAY(file_0);
		for (int n = nMeshes - 1; n >= 0; n--)
		{
			RELEASE_ARRAY(meshes[n]);
		}
		RELEASE_ARRAY(meshes);
		RELEASE_ARRAY(meshSize);
		RELEASE_ARRAY(aabbFile);
		RELEASE_ARRAY(childsSize);
		RELEASE_ARRAY(file_childs);

		// ---------------- Creating the save file and writting it -----------------------------------------

		std::string toCreate("Library/Meshes/");
		if (isChild)
		{
			toCreate += RootName;
			toCreate += "/";
		}
		else
		{
			std::string dir("Library/Meshes/");
			dir += name;
			App->fs->CreateDir(dir.data());
		}

		toCreate += name;
		toCreate += ".vgo";
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
		tmp++;
		return std::string(tmp);
}

std::string ModuleImporter::FileName(const char * file)
{
	char name[1024];
	strcpy(name, file);
	char* start = name;
	char* end = name;
	int size = 0;
	while (*start != '\0')
	{
		size++;
		start++;
	}
	end = start;
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

// Called before quitting
bool ModuleImporter::CleanUp()
{
	aiDetachAllLogStreams();

	return true;
}