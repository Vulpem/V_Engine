#include "Material.h"

#include "GameObject.h"
#include "ModuleImporter.h"
#include "Application.h"

#include "imGUI\imgui.h"

#include "OpenGL.h"

Material::Material(GameObject* linkedTo, int id) :Component(linkedTo, id)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Material##%i", id);
	name = tmp;
	type = C_material;
}

Material::~Material()
{
	std::vector<uint>::iterator it = textures.begin();
	for (; it != textures.end(); it++)
	{
		glDeleteBuffers(1, &(*it));
	}
}

void Material::EditorContent()
{
	ImGui::ColorEdit3("Color", color);

	for (uint n = 0; n < textures.size(); n++)
	{
		ImGui::Separator();
		char tmp[524];
		sprintf(tmp, "Id: %i    %s",n , texturePaths.at(n).data());
		if (ImGui::TreeNode(tmp))
		{
			ImTextureID image = (void*)textures.at(n);
			ImGui::Image(image, ImVec2(270,270));

			ImGui::TreePop();
		}		
	}
}

void Material::SaveSpecifics(pugi::xml_node& myNode)
{
	pugi::xml_node color_n = myNode.append_child("Color");
	color_n.append_attribute("R") = color[0];
	color_n.append_attribute("G") = color[1];
	color_n.append_attribute("B") = color[2];
	color_n.append_attribute("A") = color[3];
	for (std::vector<std::string>::iterator it = texturePaths.begin(); it != texturePaths.end(); it++)
	{
		pugi::xml_node tex = myNode.append_child("Texture");
		tex.append_attribute("path") = it->data();
	}

}

void Material::LoadSpecifics(pugi::xml_node & myNode)
{
	for (pugi::xml_node tex = myNode.child("Texture"); tex != nullptr; tex = tex.next_sibling())
	{
		std::string path = tex.attribute("path").as_string();
		App->importer->LoadTexture(path._Myptr(), this);
	}

	pugi::xml_node col = myNode.child("Color");

	color[0] = col.attribute("R").as_float();
	color[1] = col.attribute("G").as_float();
	color[2] = col.attribute("B").as_float();
	color[3] = col.attribute("A").as_float();

}

uint Material::NofTextures()
{
	return textures.size();
}

int Material::GetTexture(uint n)
{
	if (IsEnabled())
	{
		if (n >= 0 && n < textures.size())
		{
			return textures.at(n);
		}
	}
	return -1;
}

void Material::SetColor(float r, float g, float b, float a)
{
	color[0] = r;
	color[1] = g;
	color[2] = b;
	color[3] = a;
}

math::float4 Material::GetColor()
{
	return math::float4(color);
}