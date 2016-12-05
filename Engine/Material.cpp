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
	ImGui::Text("Blend type:");
	int alphaType = GetAlphaType();
	int prevAlphaType = alphaType;
	ImGui::RadioButton("Opaque", &alphaType, AlphaTestTypes::ALPHA_OPAQUE); ImGui::SameLine();
	ImGui::RadioButton("Discard", &alphaType, AlphaTestTypes::ALPHA_DISCARD); ImGui::SameLine();
	ImGui::RadioButton("Blend", &alphaType, AlphaTestTypes::ALPHA_BLEND);
	if (alphaType != prevAlphaType)
	{
		SetAlphaType((AlphaTestTypes)alphaType);
	}

	if (alphaType != AlphaTestTypes::ALPHA_OPAQUE)
	{
		ImGui::Text("AlphaTest:");
		float tmp = GetAlphaTest();
		if (ImGui::DragFloat("##MaterialAlphaTest", &tmp, 0.01f, 0.0f, 1.0f))
		{
			SetAlphaTest(tmp);
		}
		if (alphaType == AlphaTestTypes::ALPHA_BLEND)
		{
			if (ImGui::CollapsingHeader("Alpha Blend Types"))
			{
				int blendType = GetBlendType();
				int lastBlendType = blendType;
				ImGui::RadioButton("Zero##BlendTypes", &blendType, GL_ZERO);
				ImGui::RadioButton("One##BlendTypes", &blendType, GL_ONE);
				ImGui::RadioButton("Src_Color##BlendTypes", &blendType, GL_SRC_COLOR);
				ImGui::RadioButton("One_Minus_Src_Color##BlendTypes", &blendType, GL_ONE_MINUS_SRC_COLOR);
				ImGui::RadioButton("Dst_Color##BlendTypes", &blendType, GL_DST_COLOR);
				ImGui::RadioButton("One_Minus_Dst_Color##BlendTypes", &blendType, GL_ONE_MINUS_DST_COLOR);
				ImGui::RadioButton("Src_Alpha##BlendTypes", &blendType, GL_SRC_ALPHA);
				ImGui::RadioButton("One_Minus_Src_Alpha##BlendTypes", &blendType, GL_ONE_MINUS_SRC_ALPHA);
				ImGui::RadioButton("Dst_Alpha##BlendTypes", &blendType, GL_DST_ALPHA);
				ImGui::RadioButton("One_Minus_Dst_Alpha##BlendTypes", &blendType, GL_ONE_MINUS_DST_ALPHA);
				ImGui::RadioButton("Constant_Color##BlendTypes", &blendType, GL_CONSTANT_COLOR);
				ImGui::RadioButton("One_Minus_Constant_Color##BlendTypes", &blendType, GL_ONE_MINUS_CONSTANT_COLOR);
				ImGui::RadioButton("Constant_Alpha##BlendTypes", &blendType, GL_CONSTANT_ALPHA);
				ImGui::RadioButton("One_Minus_Constant_Alpha##BlendTypes", &blendType, GL_ONE_MINUS_CONSTANT_ALPHA);
				if (blendType != lastBlendType)
				{
					SetBlendType(blendType);
				}
			}
		}
	}

	ImGui::NewLine();
	ImGui::Separator();
	ImGui::ColorEdit4("Color", ReadRes<R_Material>()->color);

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
			R_Material* mat = ReadRes<R_Material>();
			if (mat)
			{
				R_Texture* tex = App->resources->Peek(mat->textures.at(n))->Read<R_Texture>();
				if (tex)
				{
					return tex->bufferID;
				}
			}
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

AlphaTestTypes Material::GetAlphaType()
{
	return ReadRes<R_Material>()->alphaType;
}

void Material::SetAlphaType(AlphaTestTypes type)
{
	ReadRes<R_Material>()->alphaType = type;
}

float Material::GetAlphaTest()
{
	return ReadRes<R_Material>()->alphaTest;
}

void Material::SetAlphaTest(float alphaTest)
{
	ReadRes<R_Material>()->alphaTest = alphaTest;
}

int Material::GetBlendType()
{
	return ReadRes<R_Material>()->blendType;
}

void Material::SetBlendType(int blendType)
{
	ReadRes<R_Material>()->blendType = blendType;
}
