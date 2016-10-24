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
	AABB aabb;

private:
	bool active = true;
	bool publicActive = true;
	bool hiddenOnOutliner = false;

	bool drawAABB = true;
	AABB originalAABB;
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

	void DrawLocator();
	void DrawAABB();

	//Be wary, deactivate this only for objects that the editor will take care of by itself. You won't be able to access them during runtime
	void HideFromOutliner() { hiddenOnOutliner = true; }
	bool HiddenFromOutliner() { return hiddenOnOutliner; }

	void Select(bool renderNormals = false);
	void Unselect();

	void SetOriginalAABB(float3 minPoint, float3 maxPoint);
	void UpdateAABB();

	void UpdateTransformMatrix();

	void SetActive(bool state, bool justPublic = false);
	bool IsActive() { return publicActive; }

	Component* AddComponent(Component::Type type);
	bool HasComponent(Component::Type type);

#pragma region GetComponents
	//GetComponent function
	template <typename typeComp>
	std::vector<typeComp*> GetComponent()
	{  
		std::vector<typeComp*> ret;
		std::vector<Component*>::iterator it = components.begin();
		while (it != components.end())
		{
			//Remember to add a "static GetType()" function to all created components
			if ((*it)->GetType() == typeComp::GetType())
			{
				ret.push_back((typeComp*)(*it));
			}
			it++;
		}
		return ret;
	}
#pragma endregion
};

#endif