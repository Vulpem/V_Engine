#ifndef __QUADTREE__
#define __QUADTREE__

#include "GameObject.h"

#define QUAD_GO_SIZE 1

class QuadNode
{
public:
	//Just for the root node!
	QuadNode(float3 minPoint, float3 maxPoint);
	//Default constructor
	QuadNode(QuadNode* parent);
	~QuadNode();

private:
	QuadNode* parent;
	std::vector<QuadNode> childs;
	AABB box;

	std::vector<GameObject*> GOs;
public:
	/*Try to add a GO to this Node.
	Won't be added and return false if the object's aabb doesn't collide with this node
	Will return true if it's added*/
	bool Add(GameObject* GO);
	bool Remove(GameObject* GO);

	template <typename C>
	std::vector<GameObject*> FilterCollisions(C col);

	void Draw();

	AABB GetBox() { return box; }
	void SetBox(int n, float3 breakPoint);
private:
	void CreateChilds();
	void Clean();

};


class Quad_Tree
{
public:
	Quad_Tree(float3 minPoint, float3 maxPoint);
	~Quad_Tree();

	void Add(GameObject* GO);
	void Remove(GameObject* GO);

	template <typename c>
	std::vector<GameObject*> FilterCollisions(c col);

	void Draw();
private:
	QuadNode root;

};

//QuadNode
template<typename C>
inline std::vector<GameObject*> QuadNode::FilterCollisions(C col)
{
	std::vector<GameObject*> ret;
	if (box.Intersects(col))
	{
		if (GOs.empty() == false)
		{
			for (std::vector<GameObject*>::iterator it = GOs.begin(); it != GOs.end(); it++)
			{
				if ((*it)->IsActive() && (*it)->aabb.IsFinite() && col.Intersects((*it)->aabb) == true)
				{
					ret.push_back(*it);
				}
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


//QuadTree
template<typename c>
inline std::vector<GameObject*> Quad_Tree::FilterCollisions(c col)
{
	return root.FilterCollisions(col);
}

#endif // !__QUADTREE__