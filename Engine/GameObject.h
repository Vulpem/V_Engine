#ifndef __GO__
#define __GO__

#define NAME_MAX_LEN 1024

#include "Globals.h"

#include<vector>
#include "Math.h"

#include "Component.h"
#include "Mesh.h"

class Transform;

class GameObject
{
public:
	char name[NAME_MAX_LEN];
	AABB aabb;
	OBB obb;
private:
	uint64_t uid;	

	bool active = true;
	bool publicActive = true;
	bool hiddenOnOutliner = false;
	bool Static = false;

	static const uint nComponentTypes = Component::Type::C_None;
	int HasComponents[Component::Type::C_None];

	AABB originalAABB;

	Transform* transform = nullptr;
public:

	GameObject();
	//Force the gameobject to have a certain UID. For loading purposes
	GameObject(uint64_t Uid);

	~GameObject();

	const uint64_t GetUID() { return uid; }
	std::vector<GameObject*> childs;
	GameObject* parent = nullptr;

	std::vector<Component*> components;
	bool selected = false;
	bool renderNormals = false;

	void DrawOnEditor();

	void DrawLocator();
	void DrawAABB();
	void DrawOBB();

	//Be wary, deactivate this only for objects that the editor will take care of by itself. You won't be able to access them during runtime
	void HideFromOutliner() { hiddenOnOutliner = true; }
	bool HiddenFromOutliner() { return hiddenOnOutliner; }

	void Select(bool renderNormals = false);
	void Unselect();

	void SetOriginalAABB();
	void UpdateAABB();

	void UpdateTransformMatrix();

	void SetActive(bool state, bool justPublic = false);
	bool IsActive();

	void SetStatic(bool Stat) { Static = Stat; }
	bool IsStatic() { return Static; }

	void SetName(const char* newName);
	const char* GetName();

	Component* AddComponent(Component::Type type, std::string res = std::string(""), bool forceCreation = false);

	bool HasComponent(Component::Type type);
	uint AmountOfComponent(Component::Type type);
	Transform* GetTransform();

	void Save(pugi::xml_node& node);

	//For system use, do not call please
	void RemoveComponent(Component* comp);

#pragma region GetComponents
	//GetComponent function
	template <typename typeComp>
	std::vector<typeComp*> GetComponent()
	{  
		std::vector<typeComp*> ret;
		if (HasComponent(typeComp::GetType()))
		{
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
		}
		return ret;
	}
#pragma endregion
};

#endif