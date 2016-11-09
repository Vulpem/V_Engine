#include "QuadTree.h"

#include "Application.h"
#include "ModuleRenderer3D.h"

QuadNode::QuadNode(float3 minPoint, float3 maxPoint): parent(nullptr)
{
	box.minPoint = minPoint;
	box.maxPoint = maxPoint;
}

QuadNode::QuadNode(QuadNode* _parent): parent(_parent)
{
	box = parent->GetBox();
}

QuadNode::~QuadNode()
{
}

bool QuadNode::Add(GameObject* GO)
{
	bool ret = false;
	if (box.Intersects(GO->aabb))
	{
		if (childs.empty() == true)
		{
			GOs.push_back(GO);

			if (GOs.size() > QUAD_GO_SIZE)
			{
				CreateChilds();
			}
		}
		else
		{
			std::vector<QuadNode*> collidedWith;
			for (std::vector<QuadNode>::iterator it = childs.begin(); it != childs.end(); it++)
			{
				if (it->box.Intersects(GO->aabb))
				{
					collidedWith.push_back(&*it);
				}
			}
			if (collidedWith.size() == 1)
			{
				collidedWith.front()->Add(GO);
			}
			else if (collidedWith.size() > 1)
			{
				GOs.push_back(GO);
			}
		}
		ret = true;
	}
	return ret;
}

bool QuadNode::Remove(GameObject * GO)
{
	bool ret = false;
	if (GOs.empty() == false)
	{
		for (std::vector<GameObject*>::iterator it = GOs.begin(); it != GOs.end(); it++)
		{
			if ((*it) == GO)
			{
				GOs.erase(it);
				Clean();
				return true;
			}
		}
	}

	if (childs.empty() == false)
	{
		for (std::vector<QuadNode>::iterator it = childs.begin(); it != childs.end(); it++)
		{
			ret = it->Remove(GO);
			if (ret == true)
			{
				break;
			}
		}
	}
	return ret;
}

void QuadNode::Draw()
{
	float3 corners[8];
	box.GetCornerPoints(corners);
	App->renderer3D->DrawBox(corners);
	
	for (std::vector<QuadNode>::iterator it = childs.begin(); it != childs.end(); it++)
	{
		it->Draw();
	}

}

void QuadNode::SetBox(int n, float3 breakPoint)
{
	AABB parentBox = parent->GetBox();
	switch (n)
	{
	case 0:
	{
		box.minPoint.x = parentBox.minPoint.x;
		box.minPoint.z = breakPoint.z;
		box.maxPoint.x = breakPoint.x;
		box.maxPoint.z = parentBox.maxPoint.z;
		break;
	}
	case 1:
	{
		box.minPoint.x = breakPoint.x;
		box.minPoint.z = breakPoint.z;
		box.maxPoint.x = parentBox.maxPoint.x;
		box.maxPoint.z = parentBox.maxPoint.z;
		break;
	}
	case 2:
	{
		box.minPoint.x = breakPoint.x;
		box.minPoint.z = parentBox.minPoint.z;
		box.maxPoint.x = parentBox.maxPoint.x;
		box.maxPoint.z = breakPoint.z;
		break;
	}
	case 3:
	{
		box.minPoint.x = parentBox.minPoint.x;
		box.minPoint.z = parentBox.minPoint.z;
		box.maxPoint.x = breakPoint.x;
		box.maxPoint.z = breakPoint.z;
		break;
	}
	}
}

void QuadNode::CreateChilds()
{
	float3 centerPoint = float3::zero;
	/*int n = 0;
	for (std::vector<GameObject*>::iterator it = GOs.begin(); it != GOs.end(); it++)
	{
		centerPoint.x += (*it)->aabb.CenterPoint().x;
		centerPoint.y += (*it)->aabb.CenterPoint().z;
		n++;
	}
	centerPoint /= n;

	
	//Checking if GOs collide with the X plane
	float2 newCenterPoint = centerPoint;
	for (std::vector<GameObject*>::iterator it = GOs.begin(); it != GOs.end(); it++)
	{
		while (Plane(float3(newCenterPoint.x, 0, newCenterPoint.y), float3(0, 0, 1)).Intersects((*it)->aabb) == true)
		{
			newCenterPoint.y++;
		}
	}
	//Checking if GOs collide with the Y plane
	for (std::vector<GameObject*>::iterator it = GOs.begin(); it != GOs.end(); it++)
	{
		while (Plane(float3(newCenterPoint.x, 0, newCenterPoint.y), float3(1, 0, 0)).Intersects((*it)->aabb) == true)
		{
			newCenterPoint.x++;
		}
	}

	if (newCenterPoint.x >= box.maxPoint.x || newCenterPoint.y >= box.maxPoint.z)
	{
		newCenterPoint.x = box.CenterPoint().x;
		newCenterPoint.y = box.CenterPoint().z;
	}*/
	float3 newCenterPoint = box.CenterPoint();

	for (int n = 0; n < 4; n++)
	{
		childs.push_back(QuadNode(this));
		childs.back().SetBox(n, newCenterPoint);
	}

	std::vector<GameObject*> tmp = GOs;
	GOs.clear();

	for (std::vector<GameObject*>::iterator it = tmp.begin(); it != tmp.end(); it++)
	{
		Add(*it);
	}
}

void QuadNode::Clean()
{
	bool childsHaveChilds = false;
	std::vector<GameObject*> childsGOs;
	for (std::vector<QuadNode>::iterator it = childs.begin(); it != childs.end(); it++)
	{
		if (it->childs.empty() == false)
		{
			//If a child has childs, we shouldn't erase any of them! Just in case
			childsHaveChilds = true;
			break;
		}
		for (std::vector<GameObject*>::iterator childIt = it->GOs.begin(); childIt != it->GOs.end(); childIt++)
		{
			childsGOs.push_back(*childIt);
		}
	}

	if (childsHaveChilds == false)
	{
		if (childsGOs.empty() == true)
		{
			childs.clear();
		}
		else if (childsGOs.size() + GOs.size() <= QUAD_GO_SIZE)
		{
			for (std::vector<GameObject*>::iterator it = childsGOs.begin(); it != childsGOs.end(); it++)
			{
				GOs.push_back(*it);
			}
			childs.clear();
		}

		if (parent != nullptr)
		{
			parent->Clean();
		}
	}
}


Quad_Tree::Quad_Tree(float3 minPoint, float3 maxPoint): root(minPoint, maxPoint)
{
}

Quad_Tree::~Quad_Tree()
{
}

void Quad_Tree::Add(GameObject * GO)
{
	if (GO->aabb.IsFinite())
	{
		root.Add(GO);
	}
}

void Quad_Tree::Remove(GameObject * GO)
{
	if (GO->aabb.IsFinite())
	{
		root.Remove(GO);
	}
}


void Quad_Tree::Draw()
{
	root.Draw();
}
