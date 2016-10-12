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
	if (active)
	{
		if (HasComponent(Component::Type::C_transform))
		{
			glPushMatrix();

			Transform* transform = (Transform*)(*GetComponent(Component::Type::C_transform).begin());
			glMultMatrixf(transform->GetTransformMatrix().ptr());
		}

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
		if (HasComponent(Component::Type::C_transform))
		{
			glPopMatrix();
		}
	}
}

void GameObject::DrawOnEditor()
{
	for (std::vector<Component*>::iterator it = components.begin(); it != components.end(); it++)
	{
		(*it)->DrawOnEditor();
	}
}

void GameObject::Select(bool _renderNormals)
{
	selected = true;
	renderNormals = _renderNormals;
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
	renderNormals = false;
	std::vector<GameObject*>::iterator childIt = childs.begin();
	while (childIt != childs.end())
	{
		(*childIt)->Unselect();
		childIt++;
	}
}

void GameObject::SetActive(bool state, bool justPublic)
{
	if (state == publicActive)
	{
		return;
	}
	publicActive = state;
	if (state == false)
	{
		std::vector<GameObject*>::iterator childIt = childs.begin();
		while (childIt != childs.end())
		{
			(*childIt)->SetActive(state, true);
			childIt++;
		}
	}
	if (justPublic)
	{		
		return;
	}
	active = state;

	if(state == true && parent)
	{
		parent->SetActive(true);
	}
}

Component* GameObject::AddComponent(Component::Type type)
{
	Component* toAdd = NULL;
	switch (type)
	{
	case Component::Type::C_transform:
	{
		if (HasComponent(Component::C_transform) == false)
		{
			toAdd = new Transform(this, components.size()); 
		}
		else
		{
			return *GetComponent(Component::C_transform).begin();
		}
		break;
	}
	case Component::Type::C_mesh:
	{
		toAdd = new mesh(this, components.size()); break;
	}
	case Component::Type::C_material:
	{
		if (HasComponent(Component::C_material) == false)
		{
			toAdd = new Material(this, components.size());
		}
		else
		{
			return *GetComponent(Component::C_material).begin();
		}
		break;
	}
	}

	if (toAdd != NULL)
	{
		components.push_back(toAdd);
	}
	return toAdd;
}

std::vector<Component*> GameObject::GetComponent(Component::Type type)
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

bool GameObject::HasComponent(Component::Type type)
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