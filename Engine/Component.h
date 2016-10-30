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
		C_mesh,
		C_material,
		C_camera
	};



	Component(GameObject* linkedTo, int id);
	~Component();

	virtual void Enable();
	virtual void Disable();

	void PreUpdate();
	void Update();
	void PostUpdate();
	void DrawOnEditor();

	//REMEMBER TO ADD THIS FUNCTION ALWAYS ON YOUR COMPONENTS
	Type GetType() { return type; }
	bool IsEnabled() { return enabled; }

protected:
	virtual void DoPreUpdate() {}
	virtual void DoUpdate() {}
	virtual void DoPostUpdate() {}
	virtual void DoEnable() {}
	virtual void DoDisable() {}

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