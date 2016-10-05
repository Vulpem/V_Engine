#ifndef __GO__
#define __GO__

#define NAME_MAX_LEN 1024

#include "Globals.h"

#include<vector>
#include "Math.h"

#include "Component.h"
#include "Mesh.h"

class GameObject
{
public:
	char name[NAME_MAX_LEN];

	GameObject();
	~GameObject();

	std::vector<GameObject*> childs;
	GameObject* parent = nullptr;

	std::vector<Component*> components;
	bool selected = false;

	void Update();
	void DrawOnEditor();

	void Select();
	void Unselect();

	Component* AddComponent(componentType type);
	std::vector<Component*> GetComponent(componentType type);
};

#endif