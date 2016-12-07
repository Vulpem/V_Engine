#ifndef __COMPONENT__
#define __COMPONENT__

#include "Globals.h"
#include "Math.h"

#include "PugiXml\src\pugixml.hpp"

class GameObject;

class Component
{
public:
	static const enum Type
	{
		C_GO = 1,
		C_transform,
		C_mesh,
		C_material,
		C_Texture,
		C_camera,
		C_Billboard,

		//Keep this "C_None" always last
		C_None
	};

	Component(GameObject* linkedTo, Component::Type type);
	virtual ~Component();
	
	virtual void Enable();
	virtual void Disable();

	virtual void PreUpdate() {};
	virtual void Update() {};
	virtual void PostUpdate() {};
	virtual void Draw() {};
	virtual void DrawOnEditor();

	//REMEMBER TO ADD THIS FUNCTION ALWAYS ON YOUR COMPONENTS
	Type GetType() { return type; }
	bool IsEnabled() { return enabled; }
	virtual bool MissingComponent() { return false; }

	void Save(pugi::xml_node& myNode);
protected:
	virtual void SaveSpecifics(pugi::xml_node& myNode) {}
public:
	virtual void LoadSpecifics(pugi::xml_node& myNode) {}

protected:
	virtual void OnEnable() {}
	virtual void OnDisable() {}

	virtual void EditorContent() {};

	Type type;
public:
	const uint64_t GetUID() { return uid; }

	void Delete();

	//For system use, do not call
	bool TryDeleteNow();

	std::string name;

	bool toDelete = false;

	GameObject* object;
protected:
	uint64_t uid;
private:
	bool enabled = true;
};

#endif