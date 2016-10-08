
#include "Mesh.h"

#include "OpenGL.h"
#include "imGUI\imgui.h"

#include "GameObject.h"
#include "Material.h"

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

void mesh::DoUpdate()
{
	if (init)
	{
		Draw();
	}
}

void mesh::EditorContent()
{
	ImGui::Checkbox("Wireframe", &wires);

	ImGui::Text("Vertices in memory: %i", num_vertices);
	ImGui::SameLine(ImGui::GetWindowSize().x - 90);
	ImGui::Text("Buffer: %i", id_vertices);

	ImGui::Text("Indices in memory: %i", num_indices);
	ImGui::SameLine(ImGui::GetWindowSize().x - 90);
	ImGui::Text("Buffer: %i", id_indices);

	ImGui::Text("Normals in memory: %i", num_normals);
	ImGui::SameLine(ImGui::GetWindowSize().x - 90);
	ImGui::Text("Buffer: %i", id_normals);

	ImGui::Text("UV_Coords in memory: %i", num_textureCoords);
	ImGui::SameLine(ImGui::GetWindowSize().x - 90);
	ImGui::Text("Buffer: %i", id_textureCoords);
	ImGui::Separator();
	ImGui::Text("Texture index material: %i", texMaterialIndex);
}

void mesh::Draw()
{
	bool selected = object->selected;
	if (wires == false)
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

	//Setting vertex
	glBindBuffer(GL_ARRAY_BUFFER, id_vertices);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	if (wired)
	{
		glDisable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(1.0f);
		glColor4f(0.7f, 0.7f, 0.7, 1.0f);
		if (wires)
		{
			glDisable(GL_CULL_FACE);
		}
		if(object->selected)
		{
			glLineWidth(0.1f);
			glColor4f(0, 0.8f, 0.8f, 1);
		}
	}
	else
	{
		if (object->renderNormals)
		{
			RenderNormals();
		}
		glEnable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (object->HasComponent(C_material))
		{
			Material* mat = (Material*)(*object->GetComponent(C_material).begin());
			glColor4f(mat->GetColor().x, mat->GetColor().y, mat->GetColor().z, mat->GetColor().w);
			uint textureID = mat->GetTexture(texMaterialIndex);
			if (textureID)
			{
				glBindTexture(GL_TEXTURE_2D, textureID);
			}
		}
		else
		{
			glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
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
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void mesh::RenderNormals()
{
	glDisable(GL_LIGHTING);
	// Draw Axis Grid
	glLineWidth(0.7f);

	glBegin(GL_LINES);

	glColor4f(0.54f, 0.0f, 0.54f, 1.0f);

	for (int n = 0; n < num_vertices; n++)
	{
		glVertex3f(vertices[n * 3], vertices[n * 3 + 1], vertices[n * 3 + 2]);
		
		glVertex3f(vertices[n * 3] + normals[n * 3], vertices[n * 3 + 1] + normals[n * 3 + 1], vertices[n * 3 + 2] + normals[n * 3 + 2]);
	}

	glEnd();

	glLineWidth(1.0f);
	glEnable(GL_LIGHTING);

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
		if (object->HasComponent(C_material))
		{
			Material* mat = (Material*)(*object->GetComponent(C_material).begin());

			//Keeping only the file name
			char tmp[1024] = "";
			if (path.length > 0)
			{
				
				strcpy(tmp, path.data);
				char* it = tmp;
				int size = 0;
				while (*it != '\0')
				{
					size++;
					it++;
				}
				while (*it != '/' && *it != '\\')
				{
					size--;
					it--;
					if (size <= 0)
					{
						break;
					}
				}
				
				if (size > 0)
				{
					it++;
					strcpy(tmp, it);
				}
			}
			//End

			texMaterialIndex = mat->LoadTexture(tmp);

			aiColor3D col;
			scene->mMaterials[toLoad->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, col);
			mat->SetColor(col.r, col.g, col.b);
		}

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