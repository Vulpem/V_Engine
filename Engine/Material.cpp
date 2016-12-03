#include "Material.h"

#include "GameObject.h"
#include "Application.h"

#include "R_Material.h"

#include "imGUI\imgui.h"

Material::Material(std::string res, GameObject* linkedTo, int id) : ResourcedComponent(res, linkedTo, id, C_material)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Material##%i", id);
	name = tmp;
}

void Material::EditorContent()
{
	ImGui::ColorEdit3("Color", ReadRes<R_Material>()->color);

	for (uint n = 0; n < ReadRes<R_Material>()->textures.size(); n++)
	{
		ImGui::Separator();
		char tmp[524];
		sprintf(tmp, "Id: %i    %s",n , App->resources->Peek(ReadRes<R_Material>()->textures.at(n))->name.data());
		if (ImGui::TreeNode(tmp))
		{
			ImTextureID image = (void*) App->resources->Peek(ReadRes<R_Material>()->textures.at(n))->Read<R_Texture>()->bufferID;
			ImGui::Image(image, ImVec2(270,270));

			ImGui::TreePop();
		}		
	}
}

void Material::SaveSpecifics(pugi::xml_node& myNode)
{
	pugi::xml_node color_n = myNode.append_child("Color");
	color_n.append_attribute("R") = ReadRes<R_Material>()->color[0];
	color_n.append_attribute("G") = ReadRes<R_Material>()->color[1];
	color_n.append_attribute("B") = ReadRes<R_Material>()->color[2];
	color_n.append_attribute("A") = ReadRes<R_Material>()->color[3];
	for (std::vector<uint64_t>::iterator it = ReadRes<R_Material>()->textures.begin(); it != ReadRes<R_Material>()->textures.end(); it++)
	{
		pugi::xml_node tex = myNode.append_child("Texture");		
		tex.append_attribute("path") = App->resources->Peek(ReadRes<R_Material>()->textures.at(*it))->name.data();
	}

}

void Material::LoadSpecifics(pugi::xml_node & myNode)
{
	for (pugi::xml_node tex = myNode.child("Texture"); tex != nullptr; tex = tex.next_sibling())
	{
		std::string path = tex.attribute("path").as_string();
		uint64_t toAdd = App->resources->LinkResource(path, Component::C_Texture);
		if (toAdd != 0)
		{
			ReadRes<R_Material>()->textures.push_back(toAdd);
		}
	}

	pugi::xml_node col = myNode.child("Color");

	ReadRes<R_Material>()->color[0] = col.attribute("R").as_float();
	ReadRes<R_Material>()->color[1] = col.attribute("G").as_float();
	ReadRes<R_Material>()->color[2] = col.attribute("B").as_float();
	ReadRes<R_Material>()->color[3] = col.attribute("A").as_float();

}

uint Material::NofTextures()
{
	return ReadRes<R_Material>()->textures.size();
}

int Material::GetTexture(uint n)
{
	if (IsEnabled())
	{
		if (n >= 0 && n < ReadRes<R_Material>()->textures.size())
		{
			return App->resources->Peek(ReadRes<R_Material>()->textures.at(n))->Read<R_Texture>()->bufferID;
		}
	}
	return -1;
}

void Material::SetColor(float r, float g, float b, float a)
{
	ReadRes<R_Material>()->color[0] = r;
	ReadRes<R_Material>()->color[1] = g;
	ReadRes<R_Material>()->color[2] = b;
	ReadRes<R_Material>()->color[3] = a;
}

math::float4 Material::GetColor()
{
	return math::float4(ReadRes<R_Material>()->color);
}