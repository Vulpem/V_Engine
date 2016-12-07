#include "Mesh.h"

#include "imGUI\imgui.h"

#include "GameObject.h"
#include "Mesh_RenderInfo.h"

#include "Application.h"

#include "R_mesh.h"

//------------------------- MESH --------------------------------------------------------------------------------

mesh::mesh(std::string resource, GameObject* linkedTo): ResourcedComponent(resource, linkedTo, C_mesh)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Mesh##%i", uid);
	name = tmp;

	texMaterialIndex = object->AmountOfComponent(Component::Type::C_mesh);
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

		const R_mesh* res = ReadRes<R_mesh>();

		ret.num_indices = res->num_indices;
		ret.num_vertices = res->num_vertices;

		ret.vertexBuffer = res->id_vertices;
		ret.normalsBuffer = res->id_normals;
		ret.textureCoordsBuffer = res->id_textureCoords;
		ret.indicesBuffer = res->id_indices;
	}
	return ret;
}

const float3* mesh::GetVertices() const
{
	/*//Obtaining the vertices data from the buffer
	float3* ret = new float3[num_vertices];
	glBindBuffer(GL_ARRAY_BUFFER, id_vertices);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float3) * num_vertices, ret);
	glBindBuffer(GL_ARRAY_BUFFER, 0);*/
	return ReadRes<R_mesh>()->vertices;
}

const uint mesh::GetNumVertices()
{
	return ReadRes<R_mesh>()->num_vertices;
}

const uint* mesh::GetIndices() const
{
	return ReadRes<R_mesh>()->indices;
}

const uint mesh::GetNumIndices()
{
	return ReadRes<R_mesh>()->num_indices;
}

const float3 * mesh::GetNormals() const
{
	return ReadRes<R_mesh>()->normals;
}

AABB mesh::GetAABB()
{
	return ReadRes<R_mesh>()->aabb;
}

void mesh::EditorContent()
{
	const R_mesh* res = ReadRes<R_mesh>();
	char tmp[48];
	sprintf(tmp, "Wireframe##%llu", uid);
	ImGui::Checkbox(tmp, &wires);
	ImGui::NewLine();
	ImGui::Text("Resource: %s", res->name.data());

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
	sprintf(tmp, "##MaterialID%llu", uid);
	ImGui::InputInt(tmp, &texMaterialIndex);
}

void mesh::SaveSpecifics(pugi::xml_node& myNode)
{
	Resource* res = App->resources->Peek(resource);
	myNode.append_attribute("res") = res->name.data();
	myNode.append_attribute("TextureIndex") = texMaterialIndex;
	myNode.append_attribute("Wired") = wires;
}

void mesh::LoadSpecifics(pugi::xml_node & myNode)
{
	std::string resName = myNode.attribute("res").as_string();
	resource = App->resources->LinkResource(resName.data(), GetType());

	wires = myNode.attribute("Wired").as_bool();
	texMaterialIndex = myNode.attribute("TextureIndex").as_int();
	object->SetOriginalAABB();
}
