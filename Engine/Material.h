#pragma once

#include "Component.h"

class Material :public Component
{
	Material(GameObject* linkedTo, int id);
	~Material();



};