#include "Globals.h"
#include "Application.h"

#include "ModuleImportGeometry.h"

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


//------------------------- NODE --------------------------------------------------------------------------------

Node::~Node()
{
	
	if (parent != nullptr)
	{
		std::vector<Node*>::iterator it = parent->childs.begin();
		while ((*it) != this)
		{
			it++;
		}
		parent->childs.erase(it);
	}
	else
	{
		std::vector<Node*>::iterator it = App->importGeometry->geometryNodes.begin();
		while (it != App->importGeometry->geometryNodes.end())
		{
			if ((*it) == this)
			{
				App->importGeometry->geometryNodes.erase(it);
				break;
			}
			it++;
		}
	}

	if (childs.empty() == false)
	{
		std::vector<Node*>::iterator iterator = childs.begin();
		while (childs.size() > 0 && iterator != childs.end())
		{
			delete (*iterator);
			//Erasing a Node will already remove it from the child list in its destructor, so we don't have to empty the list here, it will be done automatically
			iterator = childs.begin();
		}
	}

	if (meshes.empty() == false)
	{
		std::vector<mesh*>::iterator iterator = meshes.begin();
		while (meshes.size() > 0 && iterator != meshes.end())
		{
			delete (*iterator);
			if (meshes.size() > 1)
			{
				iterator = meshes.erase(iterator);
			}
			else
			{
				meshes.erase(iterator);
			}
			
		}
	}
}

void Node::Draw()
{
	glPushMatrix();

	math::float4x4 transform = math::float4x4::FromTRS(position, rotation, scale);
	transform.Transpose();

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

void Node::Select()
{
	std::vector<mesh*>::iterator it = meshes.begin();
	while (it != meshes.end())
	{
		(*it)->selected = true;
		it++;
	}
	std::vector<Node*>::iterator childIt = childs.begin();
	while (childIt != childs.end())
	{
		(*childIt)->Select();
		childIt++;
	}
}

void Node::Unselect()
{
	std::vector<mesh*>::iterator it = meshes.begin();
	while (it != meshes.end())
	{
		(*it)->selected = false;
		it++;
	}
	std::vector<Node*>::iterator childIt = childs.begin();
	while (childIt != childs.end())
	{
		(*childIt)->Unselect();
		childIt++;
	}
}

void Node::SetPos(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
}

void Node::ResetPos()
{
	SetPos(0, 0, 0);
}

math::float3 Node::GetPos()
{
	return position;
}

void Node::SetRot(float x, float y, float z)
{
	x *= DEGTORAD;
	y *= DEGTORAD;
	z *= DEGTORAD;
	if (x == -0) { x = 0; }
	if (y == -0) { y = 0; }
	if (z == -0) { z = 0; }
	
	rotation = math::Quat::FromEulerXYZ(x, y, z);
}

void Node::ResetRot()
{
	SetRot(0, 0, 0);
}

math::float3 Node::GetRot()
{
	math::float3 ret = rotation.ToEulerXYZ();
	ret.x *= RADTODEG;
	ret.y *= RADTODEG;
	ret.z *= RADTODEG;
	return ret;
}

void Node::SetScale(float x, float y, float z)
{
	if (x != 0 && y != 0 && z != 0)
	{
		scale.Set(x, y, z);
	}
}

void Node::ResetScale()
{
	SetScale(1, 1, 1);
}

math::float3 Node::GetScale()
{
	return scale;
}


//------------------------- MESH --------------------------------------------------------------------------------
mesh::~mesh()
{
	if (indices) { delete[] indices; }
	if (vertices) { delete[] vertices; }
	if (textureCoords) { delete[] textureCoords; }
	if (normals) { delete[] normals; }

	if (id_indices != 0)
	{
		glDeleteBuffers(1, &id_indices);
	}
	if (id_normals != 0)
	{
		glDeleteBuffers(1, &id_normals);
	}
	if (id_textureCoords != 0)
	{
		glDeleteBuffers(1, &id_textureCoords);
	}
	if (id_vertices != 0)
	{
		glDeleteBuffers(1, &id_vertices);
	}
}

void mesh::Draw()
{
	if (wires == false /*|| selected*/)
	{
		RealRender();
	}	
	if (wires == true || selected)
	{
		RealRender(true);
	}
}

void mesh::RealRender(bool wired)
{
	glEnableClientState(GL_VERTEX_ARRAY);

	glColor4f(r, g, b, a);

	//Setting vertex
	glBindBuffer(GL_ARRAY_BUFFER, id_vertices);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	if (wired)
	{
		glDisable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(4);
		glColor4f(0, 1, 1, 1);
	}
	else
	{
		glEnable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (texture != 0)
		{
			glBindTexture(GL_TEXTURE_2D, texture);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, App->importGeometry->GetCheckerID());
		}

		if (num_textureCoords > 0)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			//Setting texture coords
			glBindBuffer(GL_ARRAY_BUFFER, id_textureCoords);
			glTexCoordPointer(2, GL_FLOAT, 0, NULL);
		}

		if (num_normals > 0)
		{
			glEnableClientState(GL_NORMAL_ARRAY);
			//Setting Normals
			glBindBuffer(GL_ARRAY_BUFFER, id_normals);
			glNormalPointer(GL_FLOAT, 0, NULL);
		}
	}

	//Setting index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_indices);
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, NULL);


	//Cleaning
	glBindTexture(GL_TEXTURE_2D, 0);


	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnable(GL_LIGHTING);
}


//------------------------- MODULE --------------------------------------------------------------------------------

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

	ilInit();

	ILuint devilError = ilGetError();

	if (devilError != IL_NO_ERROR)
	{
		printf("Devil Error (ilInit: %s\n", iluErrorString(devilError));
		exit(2);
	}

		
	return ret;
}

bool ModuleImportGeometry::Start()
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
update_status ModuleImportGeometry::PreUpdate(float dt)
{
	update_status ret = UPDATE_CONTINUE;

	return ret;
}

update_status ModuleImportGeometry::Update(float dt)
{
	if (App->input->file_was_dropped)
	{
		C_String format = App->input->DroppedFileFormat();
		if ( format == ".fbx" || format == ".FBX")
		{
			LoadFBX(App->input->dropped_file);
		}
	}

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
	while (geometryNodes.size() > 0)
	{
		Node* tmp = *it;
		it = geometryNodes.erase(it);
		delete (tmp);
		
		
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

bool ModuleImportGeometry::DeleteNode(Node* toErase)
{
	std::vector<Node*>::iterator it = geometryNodes.begin();
	while (it != geometryNodes.end())
	{
		if ((*it) == toErase)
		{
			geometryNodes.erase(it);
			break;
		}
		it++;
	}

	delete toErase;

	return true;
}

uint ModuleImportGeometry::LoadTexture(char* path)
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

Node* ModuleImportGeometry::LoadNode(const aiNode* toLoad, const aiScene* scene, Node* parent)
{
	Node* ret = new Node();

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
		ret->childs.push_back(LoadNode(toLoad->mChildren[n], scene, ret));
	}

	return ret;
}

mesh* ModuleImportGeometry::LoadMesh(const aiMesh* toLoad, const aiScene* scene)
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

	toPush->texture = App->importGeometry->LoadTexture(path.data);

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

void ModuleImportGeometry::CleanName(char* toClean)
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