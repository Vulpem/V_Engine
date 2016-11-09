#ifndef __COMPONENT__
#define __COMPONENT__

#include "C_String.h"
#include "Math.h"

class GameObject;

class Component
{
public:
	enum Type
	{
		C_None,
		C_transform,
		C_material,
		C_mesh,
		C_camera
	};



	Component(GameObject* linkedTo, int id);
	virtual ~Component();

	virtual void Enable();
	virtual void Disable();

	virtual void PreUpdate() {};
	virtual void Update() {};
	virtual void PostUpdate() {};
	virtual void DrawOnEditor();

	//REMEMBER TO ADD THIS FUNCTION ALWAYS ON YOUR COMPONENTS
	Type GetType() { return type; }
	bool IsEnabled() { return enabled; }

protected:
	virtual void OnEnable() {}
	virtual void OnDisable() {}

	virtual void EditorContent() {};

	Type type;
public:
	std::string name;

	GameObject* object;
protected:
	int id = -1;
private:
	bool enabled = true;

};

#endif