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
	ImGui::ColorEdit3("Color", color);

	for (int n = 0; n < textures.size(); n++)
	{
		ImGui::Separator();
		char tmp[524];
		sprintf(tmp, "%s", texturePaths.at(n).GetString());
		if (ImGui::TreeNode(tmp))
		{
			glBindTexture(GL_TEXTURE_2D ,textures.at(n));
			ImTextureID image = (void*)textures.at(n);

			ImGui::Image(image, ImVec2(270,270));
			ImGui::TreePop();
		}		
	}
	glBindTexture(GL_TEXTURE_2D, 0);
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

	LOG("Loading Texture %s", path);

	uint ID = ilutGLLoadImage(path);


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

int Material::LoadTexture(char * path, char * file)
{
	//Checking we've actually recieved a file
	if (*file == '\0')
	{
		return -1;
	}

	char tmp[1024];
	strcpy(tmp, path);
	strcat(tmp, file);
	return LoadTexture(tmp);
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