
#include "Mesh.h"

#include "OpenGL.h"
#include "imGUI\imgui.h"

#include "GameObject.h"

#include "Assimp\include\cimport.h"
#include "Assimp\include\scene.h"
#include "Assimp\include\postprocess.h"
#include "Assimp\include\cfileio.h"

//------------------------- MESH --------------------------------------------------------------------------------

mesh::mesh(GameObject* linkedTo, int id):Component(linkedTo, id)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Mesh##%i", id);
	name = tmp;
	type = C_mesh;
}

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

void mesh::Update()
{
	if (init)
	{
		Draw();
	}
}
void mesh::EditorContent()
{
	ImGui::Text("Vertex: %i", num_indices);
	ImGui::Text("Vertex in memory: %i", num_vertices);
}

void mesh::Draw()
{
	bool selected = object->selected;
	if (wires == false || selected)
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
		glLineWidth(1.1f);
		glColor4f(0, 1, 1, 1);
	}
	else
	{
		glEnable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		/*if (texture != 0)
		{
			glBindTexture(GL_TEXTURE_2D, texture);
		}*/

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
	glBindBuffer(GL_NONE, 0);

	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnable(GL_LIGHTING);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

bool mesh::LoadMesh(const aiMesh* toLoad, const aiScene* scene)
{
	if (init == false)
	{
		glGenBuffers(1, (GLuint*) &(id_vertices));
		glGenBuffers(1, (GLuint*) &(id_indices));

		//Importing vertex
		num_vertices = toLoad->mNumVertices;
		vertices = new float[num_vertices * 3];
		memcpy_s(vertices, sizeof(float) * num_vertices * 3, toLoad->mVertices, sizeof(float) * num_vertices * 3);

		glBindBuffer(GL_ARRAY_BUFFER, id_vertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * num_vertices * 3, vertices, GL_STATIC_DRAW);

		//Importing normals
		if (toLoad->HasNormals())
		{
			glGenBuffers(1, (GLuint*) &(id_normals));
			num_normals = num_vertices;
			normals = new float[num_normals * 3];
			memcpy_s(normals, sizeof(float) * num_normals * 3, toLoad->mNormals, sizeof(float) * num_normals * 3);

			glBindBuffer(GL_ARRAY_BUFFER, id_normals);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * num_normals * 3, normals, GL_STATIC_DRAW);
		}

		//Importing texture coords
		if (toLoad->HasTextureCoords(0))
		{
			glGenBuffers(1, (GLuint*) &(id_textureCoords));
			num_textureCoords = num_vertices;

			textureCoords = new float[num_textureCoords * 2];

			aiVector3D* tmp = toLoad->mTextureCoords[0];
			for (int n = 0; n < num_textureCoords * 2; n += 2)
			{
				textureCoords[n] = tmp->x;
				textureCoords[n + 1] = tmp->y;
				tmp++;
			}

			glBindBuffer(GL_ARRAY_BUFFER, id_textureCoords);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * num_textureCoords * 2, textureCoords, GL_STATIC_DRAW);
		}
		aiString path;
		scene->mMaterials[toLoad->mMaterialIndex]->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &path);

		//Importing index (3 per face)
		if (toLoad->HasFaces())
		{
			aiFace* currentFace = toLoad->mFaces;

			num_indices = toLoad->mNumFaces * 3;
			indices = new uint[num_indices];
			for (int i = 0; i < num_indices; i += 3)
			{
				if (currentFace->mNumIndices != 3)
				{
					LOG("A loaded face had more than 3 vertices");// in %s", path);
				}
				else
				{
					indices[i] = currentFace->mIndices[0];
					indices[i + 1] = currentFace->mIndices[1];
					indices[i + 2] = currentFace->mIndices[2];
				}
				currentFace++;
			}
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_indices);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * num_indices, indices, GL_STATIC_DRAW);
		}
		init = true;
		return true;
	}
	return false;
}