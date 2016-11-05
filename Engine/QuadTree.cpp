#include "QuadTree.h"
#include "ModuleRenderer3D.h"
#include "Application.h"

QuadNode::QuadNode(QuadNode* _parent): parent(_parent)
{
	box = parent->GetBox();
	for (int n = 0; n < 4; n++)
	{
		childs[n] = nullptr;
	}
}

QuadNode::~QuadNode()
{
}

bool QuadNode::Add(GameObject* GO)
{
	bool ret = true;

	if (GOs.size() >= QUAD_GO_SIZE)
	{

	}

	return ret;
}

void QuadNode::Draw()
{
	float3 corners[8];
	box.GetCornerPoints(corners);
	App->renderer3D->DrawBox(corners);

	for (int n = 0; n < 4; n++)
	{
		if (childs[n] != nullptr)
		{
			childs[n]->Draw();
		}
	}

}

void QuadNode::SetBox(int n)
{
	AABB parentBox = parent->GetBox();
	switch (n)
	{
	case 0:
	{
		box.minPoint.x = parentBox.minPoint.x;
		box.minPoint.z = parentBox.CenterPoint().z;
		box.maxPoint.x = parentBox.CenterPoint().x;
		box.maxPoint.z = parentBox.maxPoint.z;
		break;
	}
	case 1:
	{
		box.minPoint.x = parentBox.CenterPoint().x;
		box.minPoint.z = parentBox.CenterPoint().z;
		box.maxPoint.x = parentBox.maxPoint.x;
		box.maxPoint.z = parentBox.maxPoint.z;
		break;
	}
	case 2:
	{
		box.minPoint.x = parentBox.CenterPoint().x;
		box.minPoint.z = parentBox.minPoint.z;
		box.maxPoint.x = parentBox.maxPoint.x;
		box.maxPoint.z = parentBox.CenterPoint().z;
		break;
	}
	case 3:
	{
		box.minPoint.x = parentBox.minPoint.x;
		box.minPoint.z = parentBox.minPoint.z;
		box.maxPoint.x = parentBox.CenterPoint().x;
		box.maxPoint.z = parentBox.CenterPoint().z;
		break;
	}
	}
}

void QuadNode::CreateChilds()
{
	for (int n = 0; n < 4; n++)
	{
		childs[n] = new QuadNode(this);
		childs[n]->SetBox(n);
	}
}


Quad_Tree::Quad_Tree()
{
}

Quad_Tree::~Quad_Tree()
{
}

void Quad_Tree::Draw()
{
	if (root != nullptr)
	{
		root->Draw();
	}
}
