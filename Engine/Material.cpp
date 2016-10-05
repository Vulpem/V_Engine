#include "Material.h"

#include "GameObject.h"

#include "imGUI\imgui.h"

#include "Devil\include\il.h"
#include "Devil\include\ilu.h"
#include "Devil\include\ilut.h"

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
	ImGui::Text("Color:");
	ImGui::DragFloat4("##MatColor", color, 0.005f, 0.0f, 1.0f);
	for (int n = 0; n < textures.size(); n++)
	{
		ImGui::Separator();
		char tmp[524];
		sprintf(tmp, "%s", texturePaths.at(n).GetString());
		if (ImGui::TreeNode(tmp))
		{
			//ImGui::Image();
			ImGui::TreePop();
		}		
	}
}

uint Material::NofTextures()
{
	return textures.size();
}

uint Material::GetTexture(uint n)
{
	if (n >= 0 && n < textures.size() && IsEnabled())
	{
		return textures.at(n);
	}
	else
	{
		return 0;
	}
}

int Material::LoadTexture(char* path)
{
	if (*path == '\0')
	{
		return -1;
	}

	//Checking if the texture is already loaded
	std::vector<C_String>::iterator it = texturePaths.begin();
	int n = 0;
	while (it != texturePaths.end())
	{
		if (path == it->GetString())
		{
			return textures.at(n);
		}
		it++;
		n++;
	}

	char image[256] = "FBX/";
	strcat(image, path);

	uint ID = ilutGLLoadImage(image);


	if (ID != 0)
	{
		int ret = textures.size();
		textures.push_back(ID);
		texturePaths.push_back(path);
		return ret;
	}
	else
	{
		LOG("Error loading texture %s", path);
		for (ILenum error = ilGetError(); error != IL_NO_ERROR; error = ilGetError())
		{
			LOG("devIL got error %d", error);
			//	LOG("%s", iluErrorString(error));
		}
		return -1;
	}
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