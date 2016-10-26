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

	std::vector<Component*>::iterator comp = components.begin();
	while (comp != components.end())
	{
		delete *comp;
		comp++;
	}

}

void GameObject::Update()
{
	if (active)
	{
		glPushMatrix();

		if (HasComponent(Component::Type::C_transform))
		{			
			Transform* transform = *GetComponent<Transform>().begin();
			glMultMatrixf(transform->GetGlobalTransform().ptr());
		}

		for (std::vector<Component*>::iterator it = components.begin(); it != components.end(); it++)
		{
			(*it)->Update();
		}

		if (HasComponent(Component::Type::C_mesh) == false)
		{
			DrawLocator();
		}
		glPopMatrix();

		if (drawAABB && selected)
		{
			DrawAABB();
		}

		std::vector<GameObject*>::iterator it = childs.begin();
		while (it != childs.end())
		{
			(*it)->Update();
			it++;
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

void GameObject::DrawLocator()
{
		glDisable(GL_LIGHTING);
		// Draw Axis Grid
		glLineWidth(2.0f);

		glBegin(GL_LINES);

		if (selected)
		{
			if (parent->selected)
			{
				glColor4f(0, 0.5f, 0.5f, 1);
			}
			else
			{
				glColor4f(0, 0.8f, 0.8f, 1);
			}
		}
		else
		{
			glColor4f(0.1f, 0.58f, 0.2f, 1.0f);
		}

		glVertex3f(1.0f, 0.0f, 0.0f); glVertex3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 1.0f, 0.0f); glVertex3f(0.0f, -1.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 1.0f); glVertex3f(0.0f, 0.0f, -1.0f);
		//Arrow indicating forward
		glVertex3f(0.0f, 0.0f, 1.0f); glVertex3f(0.1f, 0.0f, 0.9f);
		glVertex3f(0.0f, 0.0f, 1.0f); glVertex3f(-0.1f, 0.0f, 0.9f);

		if (childs.empty() == false)
		{
			for (std::vector<GameObject*>::iterator it = childs.begin(); it != childs.end(); it++)
			{
				if ((*it)->HasComponent(Component::Type::C_transform) && !(*it)->HasComponent(Component::Type::C_mesh))
				{
					glLineWidth(0.8f);
					math::float3 childPos((*(*it)->GetComponent<Transform>().begin())->GetLocalPos());
					glVertex3f(0.0f, 0.0f, 0.0f);
					glVertex3f(childPos.x, childPos.y, childPos.z);
				}
			}
		}

		glEnd();

		glLineWidth(1.0f);
		glEnable(GL_LIGHTING);
}

void GameObject::DrawBox(float3* corners)
{
	glDisable(GL_LIGHTING);
	glLineWidth(2.0f);
	glColor4f(1.0f, 1.0f, 0.6f, 1.0f);

	glBegin(GL_LINES);

	glVertex3fv(corners[0].ptr()); glVertex3fv(corners[1].ptr());
	glVertex3fv(corners[0].ptr()); glVertex3fv(corners[2].ptr());
	glVertex3fv(corners[0].ptr()); glVertex3fv(corners[4].ptr());
	glVertex3fv(corners[3].ptr()); glVertex3fv(corners[1].ptr());
	glVertex3fv(corners[3].ptr()); glVertex3fv(corners[2].ptr());
	glVertex3fv(corners[3].ptr()); glVertex3fv(corners[7].ptr());
	glVertex3fv(corners[5].ptr()); glVertex3fv(corners[1].ptr());
	glVertex3fv(corners[5].ptr()); glVertex3fv(corners[4].ptr());
	glVertex3fv(corners[5].ptr()); glVertex3fv(corners[7].ptr());
	glVertex3fv(corners[6].ptr()); glVertex3fv(corners[2].ptr());
	glVertex3fv(corners[6].ptr()); glVertex3fv(corners[4].ptr());
	glVertex3fv(corners[6].ptr()); glVertex3fv(corners[7].ptr());

	glEnd();

	glLineWidth(1.0f);
	glEnable(GL_LIGHTING);
}

void GameObject::DrawAABB()
{
	math::float3 corners[8];
	aabb.GetCornerPoints(corners);
	DrawBox(corners);
	
}

void GameObject::Select(bool _renderNormals)
{
	selected = true;
	renderNormals = _renderNormals;

	(*GetComponent<Transform>().begin())->UpdateEditorValues();

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

void GameObject::SetOriginalAABB(float3 minPoint, float3 maxPoint)
{
	originalAABB.minPoint = minPoint;
	originalAABB.maxPoint = maxPoint;
	UpdateAABB();
}

void GameObject::UpdateAABB()
{
	Transform* trans = *(GetComponent<Transform>().begin());
	aabb.SetNegativeInfinity();
	OBB obb = originalAABB;// .Transform(trans->GetGlobalTransform().Transposed().Float3x3Part());
	obb.Transform(trans->GetGlobalTransform().Transposed());
	aabb.Enclose(obb);
}

void GameObject::UpdateTransformMatrix()
{
	if (HasComponent(Component::Type::C_transform))
	{
		Transform* trans = *GetComponent<Transform>().begin();		
		trans->UpdateGlobalTransform();
	}

	UpdateAABB();

	if (childs.empty() == false)
	{
		std::vector<GameObject*>::iterator child = childs.begin();
		while (child != childs.end())
		{
			(*child)->UpdateTransformMatrix();
			child++;
		}
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
		break;
	}
	}

	if (toAdd != NULL)
	{
		components.push_back(toAdd);
	}
	return toAdd;
}

bool GameObject::HasComponent(Component::Type type)
{
	if (components.size() > 0)
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
	}
	return false;
}