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

	std::vector<GameObject*> FilterCollisions(float3 col);
	std::vector<GameObject*> FilterCollisions(AABB col);

	bool Collides(AABB aabb);

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

	std::vector<GameObject*> FilterCollisions(AABB col);

	void Draw();
private:
	QuadNode root;

};


#endif // !__QUADTREE__
