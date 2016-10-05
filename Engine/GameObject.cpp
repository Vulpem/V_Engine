#include "GameObject.h"

//WILL BE REMOVED WHEN WE CREATE TRANSFORM COMPONENT
#include "OpenGL.h"

//------------------------- NODE --------------------------------------------------------------------------------
GameObject::GameObject()
{

}


GameObject::~GameObject()
{

	if (parent != nullptr)
	{
		std::vector<GameObject*>::iterator it = parent->childs.begin();
		while ((*it) != this)
		{
			it++;
		}
		parent->childs.erase(it);
	}

	if (childs.empty() == false)
	{
		std::vector<GameObject*>::iterator iterator = childs.begin();
		while (childs.size() > 0 && iterator != childs.end())
		{
			delete (*iterator);
			//Erasing a Node will already remove it from the child list in its destructor, so we don't have to empty the list here, it will be done automatically
			if (childs.size() > 0)
			{
				iterator = childs.begin();
			}
		}
	}

	if (meshes.empty() == false)
	{
		std::vector<mesh*>::iterator iterator = meshes.begin();
		while (meshes.size() > 0 && iterator != meshes.end())
		{
			delete (*iterator);
			if (meshes.size() > 1)
			{
				iterator = meshes.erase(iterator);
			}
			else
			{
				meshes.erase(iterator);
			}

		}
	}
}

void GameObject::Update()
{
	for (std::vector<Component*>::iterator it = components.begin(); it != components.end(); it++)
	{
		(*it)->Update();
	}
}

void GameObject::DrawOnEditor()
{
	for (std::vector<Component*>::iterator it = components.begin(); it != components.end(); it++)
	{
		(*it)->DrawOnEditor();
	}
}

void GameObject::Draw()
{
	glPushMatrix();

	math::float4x4 transform = math::float4x4::FromTRS(position, rotation, scale);
	transform.Transpose();

	glMultMatrixf(transform.ptr());

	if (childs.empty() == false)
	{
		std::vector<GameObject*>::iterator iterator = childs.begin();
		while (iterator != childs.end())
		{
			(*iterator)->Draw();
			iterator++;
		}
	}

	if (meshes.empty() == false)
	{
		std::vector<mesh*>::iterator iterator = meshes.begin();
		while (iterator != meshes.end())
		{
			(*iterator)->Draw();
			iterator++;
		}
	}

	glPopMatrix();
}

void GameObject::Select()
{
	std::vector<mesh*>::iterator it = meshes.begin();
	while (it != meshes.end())
	{
		(*it)->selected = true;
		it++;
	}
	std::vector<GameObject*>::iterator childIt = childs.begin();
	while (childIt != childs.end())
	{
		(*childIt)->Select();
		childIt++;
	}
}

void GameObject::Unselect()
{
	std::vector<mesh*>::iterator it = meshes.begin();
	while (it != meshes.end())
	{
		(*it)->selected = false;
		it++;
	}
	std::vector<GameObject*>::iterator childIt = childs.begin();
	while (childIt != childs.end())
	{
		(*childIt)->Unselect();
		childIt++;
	}
}

void GameObject::SetPos(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
}

void GameObject::ResetPos()
{
	SetPos(0, 0, 0);
}

math::float3 GameObject::GetPos()
{
	return position;
}

void GameObject::SetRot(float x, float y, float z)
{
	x *= DEGTORAD;
	y *= DEGTORAD;
	z *= DEGTORAD;
	if (x == -0) { x = 0; }
	if (y == -0) { y = 0; }
	if (z == -0) { z = 0; }

	rotation = math::Quat::FromEulerXYZ(x, y, z);
}

void GameObject::ResetRot()
{
	SetRot(0, 0, 0);
}

math::float3 GameObject::GetRot()
{
	math::float3 ret = rotation.ToEulerXYZ();
	ret.x *= RADTODEG;
	ret.y *= RADTODEG;
	ret.z *= RADTODEG;
	return ret;
}

void GameObject::SetScale(float x, float y, float z)
{
	if (x != 0 && y != 0 && z != 0)
	{
		scale.Set(x, y, z);
	}
}

void GameObject::ResetScale()
{
	SetScale(1, 1, 1);
}

math::float3 GameObject::GetScale()
{
	return scale;
}