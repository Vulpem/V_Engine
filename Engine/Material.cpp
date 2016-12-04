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
	Resource* res = App->resources->Peek(resource);
	myNode.append_attribute("res") = res->name.data();
	pugi::xml_node color_n = myNode.append_child("Color");
	color_n.append_attribute("R") = ReadRes<R_Material>()->color[0];
	color_n.append_attribute("G") = ReadRes<R_Material>()->color[1];
	color_n.append_attribute("B") = ReadRes<R_Material>()->color[2];
	color_n.append_attribute("A") = ReadRes<R_Material>()->color[3];
}

void Material::LoadSpecifics(pugi::xml_node & myNode)
{
	std::string resName = myNode.attribute("res").as_string();
	resource = App->resources->LinkResource(resName.data(),GetType());

	pugi::xml_node col = myNode.child("Color");

	R_Material* res = ReadRes<R_Material>();
	if (res)
	{
		res->color[0] = col.attribute("R").as_float();
		res->color[1] = col.attribute("G").as_float();
		res->color[2] = col.attribute("B").as_float();
		res->color[3] = col.attribute("A").as_float();
	}
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