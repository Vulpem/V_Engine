#ifndef __COMPONENT__
#define __COMPONENT__

#include "C_String.h"
#include "Math.h"

class GameObject;

class Component
{
public:
	static const enum Type
	{
		C_transform = 1,
		C_material,
		C_mesh,
		C_camera,

		//Keep this "C_None" always last
		C_None
	};
	

	Component(GameObject* linkedTo, int id);
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