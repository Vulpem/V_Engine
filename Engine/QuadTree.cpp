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
	if (Collides(GO->aabb))
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
				if (it->Collides(GO->aabb))
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
				return true;
			}
		}
	}

	if (childs.empty() == false)
	{
		for (std::vector<QuadNode>::iterator it = childs.begin(); it != childs.end() && ret == false; it++)
		{
			ret = it->Remove(GO);
		}
	}

	if (ret == true)
	{
		Clean();
	}
	return ret;
}

std::vector<GameObject*> QuadNode::FilterCollisions(float3 col)
{
	return std::vector<GameObject*>();
}

std::vector<GameObject*> QuadNode::FilterCollisions(AABB col)
{
	std::vector<GameObject*> ret;
	if (Collides(col))
	{
		if (GOs.empty() == false)
		{
			for (std::vector<GameObject*>::iterator it = GOs.begin(); it != GOs.end(); it++)
			{
				ret.push_back(*it);
			}
		}
		if (childs.empty() == false)
		{
			for (std::vector<QuadNode>::iterator it = childs.begin(); it != childs.end(); it++)
			{
				std::vector<GameObject*> toAdd = it->FilterCollisions(col);
				if (toAdd.empty() == false)
				{
					for (std::vector<GameObject*>::iterator it = toAdd.begin(); it != toAdd.end(); it++)
					{
						ret.push_back(*it);
					}
				}
			}
		}
	}
	return ret;
}

bool QuadNode::Collides(AABB aabb)
{
	if (box.Intersects(aabb) == true)
	{
		return true;
	}
	return false;
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

void QuadNode::SetBox(int n, float2 breakPoint)
{
	AABB parentBox = parent->GetBox();
	//TMP
	breakPoint.x = parentBox.CenterPoint().x;
	breakPoint.y = parentBox.CenterPoint().z;
	////////
	switch (n)
	{
	case 0:
	{
		box.minPoint.x = parentBox.minPoint.x;
		box.minPoint.z = breakPoint.y;
		box.maxPoint.x = breakPoint.x;
		box.maxPoint.z = parentBox.maxPoint.z;
		break;
	}
	case 1:
	{
		box.minPoint.x = breakPoint.x;
		box.minPoint.z = breakPoint.y;
		box.maxPoint.x = parentBox.maxPoint.x;
		box.maxPoint.z = parentBox.maxPoint.z;
		break;
	}
	case 2:
	{
		box.minPoint.x = breakPoint.x;
		box.minPoint.z = parentBox.minPoint.z;
		box.maxPoint.x = parentBox.maxPoint.x;
		box.maxPoint.z = breakPoint.y;
		break;
	}
	case 3:
	{
		box.minPoint.x = parentBox.minPoint.x;
		box.minPoint.z = parentBox.minPoint.z;
		box.maxPoint.x = breakPoint.x;
		box.maxPoint.z = breakPoint.y;
		break;
	}
	}
}

void QuadNode::CreateChilds()
{
	float2 centerPoint = float2::zero;
	int n = 0;
	for (std::vector<GameObject*>::iterator it = GOs.begin(); it != GOs.end(); it++)
	{
		centerPoint.x += (*it)->aabb.CenterPoint().x;
		centerPoint.y += (*it)->aabb.CenterPoint().z;
		n++;
	}
	centerPoint /= n;

	for (int n = 0; n < 4; n++)
	{
		childs.push_back(QuadNode(this));
		childs.back().SetBox(n, centerPoint);
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
	std::vector<GameObject*> childsGOs;
	for (std::vector<QuadNode>::iterator it = childs.begin(); it != childs.end(); it++)
	{
		if (it->childs.empty() == false)
		{
			//If a child has childs, we shouldn't erase any of them! Just in case
			childsGOs.clear();
			break;
		}
		for (std::vector<GameObject*>::iterator childIt = it->GOs.begin(); childIt != it->GOs.end(); childIt++)
		{
			childsGOs.push_back(*childIt);
		}
	}

	if (childsGOs.empty() == false && childsGOs.size() + GOs.size() <= QUAD_GO_SIZE)
	{
		for (std::vector<GameObject*>::iterator it = childsGOs.begin(); it != childsGOs.end(); it++)
		{
			GOs.push_back(*it);
		}
		childs.clear();
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
	root.Add(GO);
}

void Quad_Tree::Remove(GameObject * GO)
{
	root.Remove(GO);
}

std::vector<GameObject*> Quad_Tree::FilterCollisions(AABB col)
{
	return root.FilterCollisions(col);
}

void Quad_Tree::Draw()
{
	root.Draw();
}
