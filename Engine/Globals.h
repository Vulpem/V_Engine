#ifndef __GLOBALS__
#define __GLOBALS__

#include <windows.h>
#include <stdio.h>

#define LOG(format, ...) log(__FILE__, __LINE__, format, __VA_ARGS__);

void log(const char file[], int line, const char* format, ...);

#define CAP(n) ((n <= 0.0f) ? n=0.0f : (n >= 1.0f) ? n=1.0f : n=n)

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f
#define HAVE_M_PI

#define EDITOR_FRAME_SAMPLES 50

#define CAP(n) ((n <= 0.0f) ? n=0.0f : (n >= 1.0f) ? n=1.0f : n=n)
#define MIN(a,b) ((a)<(b)) ? (a) : (b)
#define MAX(a,b) ((a)>(b)) ? (a) : (b)


#define RELEASE( x )\
    {\
       if( x != nullptr )\
       {\
         delete x;\
	     x = nullptr;\
       }\
    }

#define RELEASE_ARRAY( x )\
	{\
       if( x != nullptr )\
       {\
           delete[] x;\
	       x = nullptr;\
		 }\
	 }

typedef unsigned int uint;

enum update_status
{
	UPDATE_CONTINUE = 1,
	UPDATE_STOP,
	UPDATE_ERROR
};


// Configuration -----------
#define SCREEN_WIDTH 1456
#define SCREEN_HEIGHT 1024
#define SCREEN_SIZE 1
#define WIN_FULLSCREEN false
#define WIN_RESIZABLE true
#define WIN_BORDERLESS false
#define WIN_FULLSCREEN_DESKTOP false
#define VSYNC true

#define TITLE "V_Engine"
#define ORGANISATION "UPC"

#endif

/*

void ModuleImporter::ImportGameObject(const char* path, const aiNode* NodetoLoad, const aiScene* scene, bool isChild, const char* RootName)
{
const uint nMeshes = NodetoLoad->mNumMeshes;

//nMeshes
bytes = sizeof(uint);
memcpy(file_0It, &nMeshes, bytes);
file_0It += bytes;

char** meshes = new char*[nMeshes];
uint* meshSize = new uint[nMeshes];

for (int n = 0; n < nMeshes; n++)
{

meshSize[n] =
//Mesh size
sizeof(uint) +

//num_vertices							vertices								normals
sizeof(uint) + sizeof(float) * num_vertices * 3 + sizeof(float) * num_vertices * 3

//texture Coords						texture name length						tetxture name
+ sizeof(float) * num_vertices * 2 + sizeof(uint) + sizeof(char) * textureNameLen

//colors								num indices								indices
+ sizeof(float) * 3 + sizeof(uint) + sizeof(uint) * num_indices;

meshes[n] = new char[meshSize[n]];
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

uint nChilds = NodetoLoad->mNumChildren;
uint* childsSize = new uint[nChilds];
std::vector<std::string> childs;


uint childFileSize =
//nChilds			each child size
sizeof(uint) + sizeof(uint) * nChilds;


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

uint realFileSize = 0;
realFileSize += file_0Size;
for (int n = 0; n < nMeshes; n++)
{
realFileSize += meshSize[n];
}
realFileSize += childFileSize;

char* realFile = new char[realFileSize];
char* realIt = realFile;

//file_0
bytes = file_0Size;
memcpy(realIt, file_0, bytes);
childsIt += bytes;

for (int n = 0; n < nMeshes; n++)
{
//each mesh
bytes = meshSize[n];
memcpy(realIt, meshes[n], bytes);
childsIt += bytes;
}

//file_0
bytes = childFileSize;
memcpy(realIt, file_childs, bytes);
childsIt += bytes;


RELEASE_ARRAY(file_0);
for (int n = nMeshes - 1; n >= 0; n--)
{
RELEASE_ARRAY(meshes[n]);
}
RELEASE_ARRAY(meshes);
RELEASE_ARRAY(meshSize);
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
toCreate += ".vmesh";
App->fs->Save(toCreate.data(), realFile, realFileSize);

//Importing also all the childs
for (int n = 0; n < nChilds; n++)
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

RELEASE_ARRAY(realFile);

//TMP FOR LOAD AND CHECKING
/*char* toLoad = file_0;
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

}

*/