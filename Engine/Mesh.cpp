
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
	if (IsEnabled())
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