#include "Mesh.h"

#include "OpenGL.h"
#include "imGUI\imgui.h"

#include "GameObject.h"
#include "Mesh_RenderInfo.h"

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

Mesh_RenderInfo mesh::GetMeshInfo()
{
	Mesh_RenderInfo ret;
	if (object->IsActive())
	{
		if (wires == true || object->selected)
		{
			ret.wired = true;
			if (wires == true)
			{
				ret.doubleSidedFaces = true;
				ret.wiresColor = float4(0.f, 0.f, 0.f, 1.0f);
			}
			if (object->selected)
			{
				if (object->parent && object->parent->selected)
				{
					ret.wiresColor = float4(0, 0.5f, 0.5f, 1);
				}
				else
				{
					ret.wiresColor = float4(0, 0.8f, 0.8f, 1);
				}
			}
		}
		if (wires == false)
		{
			ret.filled = true;
		}

		ret.renderNormals = object->renderNormals;

		ret.num_indices = num_indices;
		ret.num_vertices = num_vertices;

		ret.vertexBuffer = id_vertices;
		ret.normalsBuffer = id_normals;
		ret.textureCoordsBuffer = id_textureCoords;
		ret.indicesBuffer = id_indices;
	}
	return ret;
}

float3 * mesh::GetVertices()
{
	//Obtaining the vertices data from the buffer
	float3* ret = new float3[num_vertices];
	glBindBuffer(GL_ARRAY_BUFFER, id_vertices);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float3) * num_vertices, ret);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return ret;
}

uint* mesh::GetIndices()
{
	//Obtaining the index data from the buffer
	uint* ret = new uint[num_indices];
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_indices);
	glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint) * num_indices, ret);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return ret;
}

float3 * mesh::GetNormals()
{
	//Obtaining the normals data from the buffer
	float3* ret = new float3[num_normals];
	glBindBuffer(GL_ARRAY_BUFFER, id_normals);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float3) * num_normals, ret);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return ret;
}

void mesh::EditorContent()
{
	char tmp[48];
	sprintf(tmp, "Wireframe##%i", id);
	ImGui::Checkbox(tmp, &wires);

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