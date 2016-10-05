#pragma once

#include "C_String.h"
#include "Math.h"

enum componentType
{
	C_transform,
	C_mesh,
	C_material
};

class GameObject;

class Component
{
public:
	Component(GameObject* linkedTo, int id);
	~Component();

	virtual void Enable();
	virtual void Disable();

	void Update();
	void DrawOnEditor();

	componentType GetType();
	bool IsEnabled() { return enabled; }

protected:
	virtual void DoUpdate() {}
	virtual void DoEnable() {}
	virtual void DoDisable() {}

	virtual void EditorContent() {};

	componentType type;
public:
	C_String name;

	GameObject* object;
protected:
	int id = -1;
private:
	bool enabled = true;

};