#include "Material.h"

#include "GameObject.h"

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

}

void Material::EditorContent()
{
	ImGui::ColorEdit3("Color", color);

	for (int n = 0; n < textures.size(); n++)
	{
		ImGui::Separator();
		char tmp[524];
		sprintf(tmp, "Id: %i    %s",n , texturePaths.at(n).GetString());
		if (ImGui::TreeNode(tmp))
		{
			glBindTexture(GL_TEXTURE_2D ,textures.at(n));
			ImTextureID image = (void*)textures.at(n);

			ImGui::Image(image, ImVec2(270,270));
			glBindTexture(GL_TEXTURE_2D, 0);
			ImGui::TreePop();
		}		
	}
}

uint Material::NofTextures()
{
	return textures.size();
}

int Material::GetTexture(uint n)
{
	if (IsEnabled())
	{
		if (n >= 0 && n < textures.size() && IsEnabled())
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