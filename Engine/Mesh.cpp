#include "Mesh.h"

#include "OpenGL.h"
#include "imGUI\imgui.h"

#include "GameObject.h"
#include "Mesh_RenderInfo.h"

#include "Application.h"
#include "ModuleImporter.h"

#include "ModuleResourceManager.h"

//------------------------- MESH --------------------------------------------------------------------------------

mesh::mesh(std::string resource, GameObject* linkedTo, int id):ResourceComponent(resource, linkedTo, id)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Mesh##%i", id);
	name = tmp;
	type = C_mesh;

	texMaterialIndex = object->AmountOfComponent(Component::Type::C_mesh);
}

mesh::~mesh()
{
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

		ret.num_indices = resource->num_indices;
		ret.num_vertices = resource->num_vertices;

		ret.vertexBuffer = resource->id_vertices;
		ret.normalsBuffer = resource->id_normals;
		ret.textureCoordsBuffer = resource->id_textureCoords;
		ret.indicesBuffer = resource->id_indices;
	}
	return ret;
}

const float3 * mesh::GetVertices() const
{
	/*//Obtaining the vertices data from the buffer
	float3* ret = new float3[num_vertices];
	glBindBuffer(GL_ARRAY_BUFFER, id_vertices);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float3) * num_vertices, ret);
	glBindBuffer(GL_ARRAY_BUFFER, 0);*/
	return resource->vertices;
}

const uint* mesh::GetIndices() const
{
	return resource->indices;
}

const float3 * mesh::GetNormals() const
{
	return resource->normals;
}

void mesh::EditorContent()
{
	char tmp[48];
	sprintf(tmp, "Wireframe##%i", id);
	ImGui::Checkbox(tmp, &wires);
	ImGui::NewLine();
	ImGui::Text("Resource: %s", resource->file);

	ImGui::Text("Vertices in memory: %i", resource->num_vertices);
	ImGui::SameLine(ImGui::GetWindowSize().x - 90);
	ImGui::Text("Buffer: %i", resource->id_vertices);

	ImGui::Text("Indices in memory: %i", resource->num_indices);
	ImGui::SameLine(ImGui::GetWindowSize().x - 90);
	ImGui::Text("Buffer: %i", resource->id_indices);

	ImGui::Text("Normals in memory: %i", resource->num_normals);
	ImGui::SameLine(ImGui::GetWindowSize().x - 90);
	ImGui::Text("Buffer: %i", resource->id_normals);

	ImGui::Text("UV_Coords in memory: %i", resource->num_textureCoords);
	ImGui::SameLine(ImGui::GetWindowSize().x - 90);
	ImGui::Text("Buffer: %i", resource->id_textureCoords);
	ImGui::Separator();
	ImGui::Text("Texture index material:");
	sprintf(tmp, "##MaterialID%i", id);
	ImGui::InputInt(tmp, &texMaterialIndex);
}

void mesh::SaveSpecifics(pugi::xml_node& myNode)
{
	myNode.append_attribute("MeshPath") = resource->file.data();
	myNode.append_attribute("TextureIndex") = texMaterialIndex;
}