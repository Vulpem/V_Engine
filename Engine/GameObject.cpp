#include "GameObject.h"

#include "AllComponents.h"

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
}

void GameObject::Update()
{
	glPushMatrix();

	Transform* tmp2 = (Transform*)(*GetComponent(C_transform).begin());
	glMultMatrixf(tmp2->GetTransformMatrix().ptr());

	for (std::vector<Component*>::iterator it = components.begin(); it != components.end(); it++)
	{
		(*it)->Update();
	}

	std::vector<GameObject*>::iterator it = childs.begin();
	while (it != childs.end())
	{
		(*it)->Update();
		it++;
	}

	glPopMatrix();
}

void GameObject::DrawOnEditor()
{
	for (std::vector<Component*>::iterator it = components.begin(); it != components.end(); it++)
	{
		(*it)->DrawOnEditor();
	}
}

void GameObject::Select()
{
	selected = true;
	std::vector<GameObject*>::iterator childIt = childs.begin();
	while (childIt != childs.end())
	{
		(*childIt)->Select();
		childIt++;
	}
}

void GameObject::Unselect()
{
	selected = false;
	std::vector<GameObject*>::iterator childIt = childs.begin();
	while (childIt != childs.end())
	{
		(*childIt)->Unselect();
		childIt++;
	}
}

Component* GameObject::AddComponent(componentType type)
{
	Component* toAdd = NULL;
	switch (type)
	{
	case C_transform:
	{
		toAdd = new Transform(this, components.size()); break;
	}
	case C_mesh:
	{
		toAdd = new mesh(this, components.size()); break;
	}
	case C_material:
	{
		toAdd = new Material(this, components.size()); break;
	}
	}
	if (toAdd != NULL)
	{
		components.push_back(toAdd);
	}
	return toAdd;
}

std::vector<Component*> GameObject::GetComponent(componentType type)
{
	std::vector<Component*> ret;
	std::vector<Component*>::iterator it = components.begin();
	while (it != components.end())
	{
		if ((*it)->GetType() == type)
		{
			ret.push_back((*it));			
		}
		it++;
	}
	return ret;
}

bool GameObject::HasComponent(componentType type)
{
	std::vector<Component*>::iterator it = components.begin();
	while (it != components.end())
	{
		if ((*it)->GetType() == type)
		{
			return true;
		}
		it++;
	}
	return false;
}