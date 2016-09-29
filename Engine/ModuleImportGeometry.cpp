#include "Globals.h"
#include "Application.h"

#include "ModuleImportGeometry.h"

#include "ModuleFileSystem.h"

#include "OpenGL.h"


#include "Assimp\include\cimport.h"
#include "Assimp\include\scene.h"
#include "Assimp\include\postprocess.h"
#include "Assimp\include\cfileio.h"

#pragma comment(lib, "Assimp/libx86/assimp.lib")

Node::~Node()
{
	if (childs.empty() == false)
	{
		std::vector<Node*>::iterator iterator = childs.begin();
		while (iterator != childs.end())
		{
			delete (*iterator);
			iterator++;
		}
	}

	if (meshes.empty() == false)
	{
		std::vector<mesh*>::iterator iterator = meshes.begin();
		while (iterator != meshes.end())
		{
			delete (*iterator);
			iterator++;
		}
	}
}

void Node::Draw()
{
	glPushMatrix();
	glMultMatrixf(transform.ptr());

	if (childs.empty() == false)
	{
		std::vector<Node*>::iterator iterator = childs.begin();
		while (iterator != childs.end())
		{
			(*iterator)->Draw();
			iterator++;
		}
	}

	if (meshes.empty() == false)
	{
		std::vector<mesh*>::iterator iterator = meshes.begin();
		while (iterator != meshes.end())
		{
			(*iterator)->Draw();
			iterator++;
		}
	}

	glPopMatrix();
}

void Node::SetPos(float x, float y, float z)
{
	transform.Translate(x, y, z);
}

void mesh::Draw()
{
	if (wires)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	glEnableClientState(GL_VERTEX_ARRAY);

	glColor4f(r, g, b, a);

	//Setting vertex
	glBindBuffer(GL_ARRAY_BUFFER, id_vertices);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	if (num_normals > 0)
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		//Setting Normals
		glBindBuffer(GL_ARRAY_BUFFER, id_normals);
		glNormalPointer(GL_FLOAT, 0, NULL);
	}

	//Setting index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_indices);
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, NULL);

	//Cleaning
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

ModuleImportGeometry::ModuleImportGeometry(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name.create("ModuleGeometry");
}

// DestructorF
ModuleImportGeometry::~ModuleImportGeometry()
{

}

// Called before render is available
bool ModuleImportGeometry::Init()
{
	bool ret = true;

	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);
		
	return ret;
}

bool ModuleImportGeometry::Start()
{
	
	return true;
}

// Called every draw update
update_status ModuleImportGeometry::PreUpdate(float dt)
{
	update_status ret = UPDATE_CONTINUE;

	return ret;
}

update_status ModuleImportGeometry::Update(float dt)
{
	
	std::vector<Node*>::iterator it = geometryNodes.begin();
	while (it != geometryNodes.end())
	{
		(*it)->Draw();
		it++;
	}

	return UPDATE_CONTINUE;
}

update_status ModuleImportGeometry::PostUpdate(float dt)
{
	
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleImportGeometry::CleanUp()
{
	aiDetachAllLogStreams();

	std::vector<Node*>::iterator it = geometryNodes.begin();
	while (it != geometryNodes.end())
	{
		delete (*it);
		it++;
	}

	return true;
}

Node* ModuleImportGeometry::LoadFBX(char* path)
{
//	SDL_RWops* file = App->fs->Load(path);

//	aiFileIO aiFile;
//	aiFile.OpenProc(&aiFile, "App->fs->Load(path)", "this");

	Node* ret = NULL;

	const aiScene* scene = aiImportFile(path, aiProcessPreset_TargetRealtime_MaxQuality);
	if (scene != nullptr)
	{
		if (scene->HasMeshes())
		{
			ret = LoadNode(scene->mRootNode, scene);
			geometryNodes.push_back(ret);
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

Node* ModuleImportGeometry::LoadNode(const aiNode* toLoad, const aiScene* scene, Node* parent)
{
	Node* ret = new Node();

	//Setting Name
	char tmpName[MAXLEN];
	memcpy(tmpName, toLoad->mName.data, toLoad->mName.length + 1);
	ret->name = tmpName;

	//Setting parent
	ret->parent = parent;

	//Setting transform
	ret->transform.SetIdentity();

	ret->transform[0][0] = toLoad->mTransformation.a1;
	ret->transform[0][1] = toLoad->mTransformation.b1;
	ret->transform[0][2] = toLoad->mTransformation.c1;
	ret->transform[0][3] = toLoad->mTransformation.d1;

	ret->transform[1][0] = toLoad->mTransformation.a2;
	ret->transform[1][1] = toLoad->mTransformation.b2;
	ret->transform[1][2] = toLoad->mTransformation.c2;
	ret->transform[1][3] = toLoad->mTransformation.d2;

	ret->transform[2][0] = toLoad->mTransformation.a3;
	ret->transform[2][1] = toLoad->mTransformation.b3;
	ret->transform[2][2] = toLoad->mTransformation.c3;
	ret->transform[2][3] = toLoad->mTransformation.d3;

	ret->transform[3][0] = toLoad->mTransformation.a4;
	ret->transform[3][1] = toLoad->mTransformation.b4;
	ret->transform[3][2] = toLoad->mTransformation.c4;
	ret->transform[3][3] = toLoad->mTransformation.d4;


	//Loading meshes
	for (int n = 0; n < toLoad->mNumMeshes; n++)
	{   
		mesh* tmp = LoadMesh(scene->mMeshes[toLoad->mMeshes[n]]);
		ret->meshes.push_back(tmp);
	}

	//Loading child nodes
	for (int n = 0; n < toLoad->mNumChildren; n++)
	{
		ret->childs.push_back(LoadNode(toLoad->mChildren[n], scene, ret));
	}

	return ret;
}

mesh* ModuleImportGeometry::LoadMesh(const aiMesh* toLoad)
{
	mesh* toPush = new mesh;

	char tmpName[MAXLEN];
	memcpy(tmpName, toLoad->mName.data, toLoad->mName.length + 1);
	toPush->name = tmpName;

	glGenBuffers(1, (GLuint*) &(toPush->id_vertices));
	glGenBuffers(1, (GLuint*) &(toPush->id_indices));
	glGenBuffers(1, (GLuint*) &(toPush->id_normals));

	//Importing vertex
	toPush->num_vertices = toLoad->mNumVertices;
	toPush->vertices = new float[toPush->num_vertices * 3];
	memcpy_s(toPush->vertices, sizeof(float) * toPush->num_vertices * 3, toLoad->mVertices, sizeof(float) * toPush->num_vertices * 3);

	glBindBuffer(GL_ARRAY_BUFFER, toPush->id_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * toPush->num_vertices * 3, toPush->vertices, GL_STATIC_DRAW);

	//Importing normals
	if (toLoad->HasNormals())
	{
		toPush->num_normals = toPush->num_vertices;
		toPush->normals = new float[toPush->num_normals * 3];
		memcpy_s(toPush->normals, sizeof(float) * toPush->num_normals * 3, toLoad->mNormals, sizeof(float) * toPush->num_normals * 3);

		glBindBuffer(GL_ARRAY_BUFFER, toPush->id_normals);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * toPush->num_normals * 3, toPush->normals, GL_STATIC_DRAW);
	}


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