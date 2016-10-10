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
private:
	bool active = true;
	bool publicActive = true;
	bool hiddenOnOutliner = false;
public:

	GameObject();
	~GameObject();

	std::vector<GameObject*> childs;
	GameObject* parent = nullptr;

	std::vector<Component*> components;
	bool selected = false;
	bool renderNormals = false;

	void Update();
	void DrawOnEditor();

	//Be wary, deactivate this only for objects that the editor will take care of by itself. You won't be able to access them during runtime
	void HideFromOutliner() { hiddenOnOutliner = true; }
	bool HiddenFromOutliner() { return hiddenOnOutliner; }

	void Select(bool renderNormals = false);
	void Unselect();

	void SetActive(bool state, bool justPublic = false);
	bool IsActive() { return publicActive; }

	Component* AddComponent(Component::Type type);
	std::vector<Component*> GetComponent(Component::Type type);
	bool HasComponent(Component::Type type);
};

#endif