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


void mesh::Draw()
{
	glPushMatrix();
	glMultMatrixf(transform.ptr());

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

	glPopMatrix();

}

void mesh::SetPos(float x, float y, float z)
{
	transform.Translate(x, y, z);
}

ModuleImportGeometry::ModuleImportGeometry(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name.create("ModuleGeometry");
}

// Destructor
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
	
	std::vector<mesh*>::iterator it = meshes.begin();
	while (it != meshes.end())
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

	return true;
}

mesh* ModuleImportGeometry::LoadFBX(char* path)
{
//	SDL_RWops* file = App->fs->Load(path);

//	aiFileIO aiFile;
//	aiFile.OpenProc(&aiFile, "App->fs->Load(path)", "this");
	mesh* ret = NULL;

	const aiScene* scene = aiImportFile(path, aiProcessPreset_TargetRealtime_MaxQuality);
	if (scene != nullptr)
	{
		if (scene->HasMeshes())
		{
			for (uint n = 0; n < scene->mNumMeshes; n++)
			{
				const aiMesh* impMesh = scene->mMeshes[n];
	
				

				meshes.push_back(LoadMesh(impMesh, path));
			}
		}
		if (scene)
		{
			aiReleaseImport(scene);
			scene = NULL;
		}

		ret = (*meshes.cbegin());

	}
	else
	{
		LOG("Error loading scene %s", path);
	}

	return ret;
}

mesh* ModuleImportGeometry::LoadMesh(const aiMesh* toLoad, char* path)
{
	mesh* toPush = new mesh;

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
				LOG("A loaded face had more than 3 vertices in %s", path);
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