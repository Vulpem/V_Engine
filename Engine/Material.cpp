#include "Material.h"

#include "GameObject.h"
#include "ModuleImporter.h"
#include "Application.h"

#include "ModuleResourceManager.h"
#include "R_Material.h"

#include "imGUI\imgui.h"

#include "OpenGL.h"

Material::Material(std::string res, GameObject* linkedTo, int id) : ResourceComponent(res, linkedTo, id, C_material)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Material##%i", id);
	name = tmp;
}

void Material::EditorContent()
{
	ImGui::ColorEdit3("Color", resource->Read<R_Material>()->color);

	for (uint n = 0; n < resource->Read<R_Material>()->textures.size(); n++)
	{
		ImGui::Separator();
		char tmp[524];
		sprintf(tmp, "Id: %i    %s",n , resource->Read<R_Material>()->textures.at(n)->file.data());
		if (ImGui::TreeNode(tmp))
		{
			ImTextureID image = (void*)resource->Read<R_Material>()->textures.at(n)->bufferID;
			ImGui::Image(image, ImVec2(270,270));

			ImGui::TreePop();
		}		
	}
}

void Material::SaveSpecifics(pugi::xml_node& myNode)
{
	pugi::xml_node color_n = myNode.append_child("Color");
	color_n.append_attribute("R") = resource->Read<R_Material>()->color[0];
	color_n.append_attribute("G") = resource->Read<R_Material>()->color[1];
	color_n.append_attribute("B") = resource->Read<R_Material>()->color[2];
	color_n.append_attribute("A") = resource->Read<R_Material>()->color[3];
	for (std::vector<R_Texture*>::iterator it = resource->Read<R_Material>()->textures.begin(); it != resource->Read<R_Material>()->textures.end(); it++)
	{
		pugi::xml_node tex = myNode.append_child("Texture");		
		tex.append_attribute("path") = (*it)->file.data();
	}

}

void Material::LoadSpecifics(pugi::xml_node & myNode)
{
	for (pugi::xml_node tex = myNode.child("Texture"); tex != nullptr; tex = tex.next_sibling())
	{
		std::string path = tex.attribute("path").as_string();
		R_Texture* toAdd = (R_Texture*)App->resources->LinkResource(path, Component::C_Texture);
		if (toAdd != nullptr)
		{
			resource->Read<R_Material>()->textures.push_back(toAdd);
		}
	}

	pugi::xml_node col = myNode.child("Color");

	resource->Read<R_Material>()->color[0] = col.attribute("R").as_float();
	resource->Read<R_Material>()->color[1] = col.attribute("G").as_float();
	resource->Read<R_Material>()->color[2] = col.attribute("B").as_float();
	resource->Read<R_Material>()->color[3] = col.attribute("A").as_float();

}

uint Material::NofTextures()
{
	return resource->Read<R_Material>()->textures.size();
}

int Material::GetTexture(uint n)
{
	if (IsEnabled())
	{
		if (n >= 0 && n < resource->Read<R_Material>()->textures.size())
		{
			return resource->Read<R_Material>()->textures.at(n)->bufferID;
		}
	}
	return -1;
}

void Material::SetColor(float r, float g, float b, float a)
{
	resource->Read<R_Material>()->color[0] = r;
	resource->Read<R_Material>()->color[1] = g;
	resource->Read<R_Material>()->color[2] = b;
	resource->Read<R_Material>()->color[3] = a;
}

math::float4 Material::GetColor()
{
	return math::float4(resource->Read<R_Material>()->color);
}