#pragma once

#include "C_String.h"

enum componentType
{
	C_transform,
	C_mesh,
	C_material
};


class Component
{
public:
	Component();
	~Component();

	virtual void Enable();
	virtual void Disable();

	virtual void Update() { }
	void DrawOnEditor();

	componentType GetType();

protected:
	virtual void Activate() {}
	virtual void Deactivate() {}

	virtual void EditorContent() {};

public:
	C_String name;
private:
	componentType type;
	bool enabled = true;

};