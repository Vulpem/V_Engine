#include "Mesh.h"

#include "OpenGL.h"
#include "imGUI\imgui.h"

#include "GameObject.h"
#include "Mesh_RenderInfo.h"

#include "Application.h"
#include "ModuleImporter.h"

#include "ModuleResourceManager.h"

//------------------------- MESH --------------------------------------------------------------------------------

mesh::mesh(std::string resource, GameObject* linkedTo, int id): ResourceComponent(resource, linkedTo, id, C_mesh)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Mesh##%i", id);
	name = tmp;

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

		const R_mesh* res = resource->Read<R_mesh>();

		ret.num_indices = res->num_indices;
		ret.num_vertices = res->num_vertices;

		ret.vertexBuffer = res->id_vertices;
		ret.normalsBuffer = res->id_normals;
		ret.textureCoordsBuffer = res->id_textureCoords;
		ret.indicesBuffer = res->id_indices;
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
	return resource->Read<R_mesh>()->vertices;
}

const uint mesh::GetNumVertices()
{
	return resource->Read<R_mesh>()->num_vertices;
}

const uint* mesh::GetIndices() const
{
	return resource->Read<R_mesh>()->indices;
}

const uint mesh::GetNumIndices()
{
	return resource->Read<R_mesh>()->num_indices;
}

const float3 * mesh::GetNormals() const
{
	return resource->Read<R_mesh>()->normals;
}

AABB mesh::GetAABB()
{
	return resource->Read<R_mesh>()->aabb;
}

void mesh::EditorContent()
{
	const R_mesh* res = resource->Read<R_mesh>();
	char tmp[48];
	sprintf(tmp, "Wireframe##%i", id);
	ImGui::Checkbox(tmp, &wires);
	ImGui::NewLine();
	ImGui::Text("Resource: %s", resource->file.data());

	ImGui::Text("Vertices in memory: %i", res->num_vertices);
	ImGui::SameLine(ImGui::GetWindowSize().x - 90);
	ImGui::Text("Buffer: %i", res->id_vertices);

	ImGui::Text("Indices in memory: %i", res->num_indices);
	ImGui::SameLine(ImGui::GetWindowSize().x - 90);
	ImGui::Text("Buffer: %i", res->id_indices);

	ImGui::Text("Normals in memory: %i", res->num_normals);
	ImGui::SameLine(ImGui::GetWindowSize().x - 90);
	ImGui::Text("Buffer: %i", res->id_normals);

	ImGui::Text("UV_Coords in memory: %i", res->num_textureCoords);
	ImGui::SameLine(ImGui::GetWindowSize().x - 90);
	ImGui::Text("Buffer: %i", res->id_textureCoords);
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