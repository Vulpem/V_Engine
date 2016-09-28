#include "Globals.h"
#include "Application.h"

#include "ModuleImportGeometry.h"

#include "OpenGL.h"


#include "Assimp\include\cimport.h"
#include "Assimp\include\scene.h"
#include "Assimp\include\postprocess.h"
#include "Assimp\include\cfileio.h"

#pragma comment(lib, "Assimp/libx86/assimp.lib")


void mesh::Draw()
{
	if (wires)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	glEnableClientState(GL_VERTEX_ARRAY);

	glColor4f(r, g, b, a);

	glBindBuffer(GL_ARRAY_BUFFER, id_vertices);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_indices);
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, NULL);

	glDisableClientState(GL_VERTEX_ARRAY);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

ModuleImportGeometry::ModuleImportGeometry(Application* app, bool start_enabled) : Module(app, start_enabled)
{

}

// Destructor
ModuleImportGeometry::~ModuleImportGeometry()
{

}

// Called before render is available
bool ModuleImportGeometry::Init()
{
	bool ret = true;

	//struct aiLogStream stream;
	//stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	//aiAttachLogStream(&stream);
		
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
	//aiDetachAllLogStreams();

	return true;
}

void ModuleImportGeometry::LoadFBX(char* path)
{
	const aiScene* scene = aiImportFile(path, aiProcessPreset_TargetRealtime_MaxQuality);
	if (scene != nullptr)
	{
		if (scene->HasMeshes())
		{
			for (uint n = 0; n < scene->mNumMeshes; n++)
			{
				const aiMesh* impMesh = scene->mMeshes[n];
	
				mesh* toPush = new mesh;

				glGenBuffers(1, (GLuint*) &(toPush->id_vertices));
				glGenBuffers(1, (GLuint*) &(toPush->id_indices));

				//Importing vertex
				toPush->num_vertices = impMesh->mNumVertices;
				toPush->vertices = new float[toPush->num_vertices * 3];
				memcpy_s(toPush->vertices, sizeof(float) * toPush->num_vertices * 3, impMesh->mVertices, sizeof(float) * toPush->num_vertices * 3);

				glBindBuffer(GL_ARRAY_BUFFER, toPush->id_vertices);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * toPush->num_vertices * 3, toPush->vertices, GL_STATIC_DRAW);


				//Importing index (3 per face)
				if (impMesh->HasFaces())
				{
					aiFace* currentFace = impMesh->mFaces;

					toPush->num_indices = impMesh->mNumFaces * 3;
					toPush->indices = new uint[toPush->num_indices];
					for (int i = 0; i < toPush->num_indices; i += 3)
					{
						if (currentFace->mNumIndices != 3)
						{
							LOG("A face had more than 3 vertices in %s", path);
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
				meshes.push_back(toPush);
			}
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
}