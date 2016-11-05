#ifndef __QUADTREE__
#define __QUADTREE__

#include "GameObject.h"

#define QUAD_GO_SIZE 1

class QuadNode
{
public:
	QuadNode(QuadNode* parent);
	~QuadNode();

private:
	QuadNode* parent;
	QuadNode* childs[4];
	AABB box;

	std::vector<GameObject*> GOs;
public:
	/*Try to add a GO to this Node.
	Won't be added and return false if the object's aabb doesn't collide with this node
	Will return true if it's added*/
	bool Add(GameObject* GO);

	void Draw();

	AABB GetBox() { return box; }
	void SetBox(int n);
private:
	void CreateChilds();

};


class Quad_Tree
{
public:
	Quad_Tree();
	~Quad_Tree();

	void Draw();
private:
	QuadNode* root = nullptr;

};


#endif // !__QUADTREE__
