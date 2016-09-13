#include "Tower.h"

Tower::Tower()
{}
Tower::Tower(float x, float y, float z, TOWER_TYPE newType, float height, float cubeSize)
{
	type = newType;
	for (int h = 0; h < height; h++)
	{
		int w = 0;
		Cube* cube = new Cube(cubeSize, cubeSize, cubeSize);
		float offsetX = 0;
		float offsetZ = 0;
		if (w == 1 || w == 3)
			offsetX = cubeSize;
		if (w > 1)
			offsetZ = cubeSize;
		int posY = y + cubeSize / 2;

		cube->SetPos(x, posY + h*cubeSize, z);
		positions.PushBack({ x, posY + h*cubeSize, z });
		cube->color = GetColor(type);

		cubes.PushBack(cube);
	}

}
bool Tower::Update()
{
	bool ret = true;
	for (int i = 0; i < cubes.Count(); i++)
	{
		pbs[i]->GetTransform(&cubes[i]->transform);
		cubes[i]->Render();
	}
	if (!fallen)
	{
		if (isDestroyed())
		{
			ChangeColor(Color{ 0.5f, 0.5f, 0.5f });
			fallen = true;
			ret = false;
		}
	}
	return ret;
}

bool Tower::isDestroyed()
{
	int blocksDown = 0;
	for (int i = 0; i < pbs.Count(); i++)
	{
		float x, y, z;
		pbs[i]->GetPos(&x, &y, &z);
		float difX = (x - positions[i].x);
		if (difX < 0)
		{
			difX *= -1;
		}

		float difY = (y - positions[i].y);
		if (difY < 0)
		{
			difY *= -1;
		}

		float difZ = (z - positions[i].z);
		if (difZ < 0)
		{
			difZ *= -1;
		}

		float distance = difX + difY + difZ;
		if (distance > 3.0f)
		{
			blocksDown++;
			cubes[i]->color = Color{ 0.5f, 0.5f, 0.5f };
		}

	}
	if (blocksDown > pbs.Count() / 3 + 0.5)
		return true;
	return false;
}

void Tower::ChangeColor(Color color)
{
	for (int i = 0; i < cubes.Count(); i++)
	{
		cubes[i]->color = color;
	}
}

Color Tower::GetColor(TOWER_TYPE type)
{
	Color color;
	switch (type)
	{
	case(TOWER_ALLY):
		color = { 0.0f, 0.5f, 0.8f };
		break;
	case(TOWER_ENEMY) :
		color = { 0.8f, 0.3f, 0.0f };
		break;
	case(TOWER_NEUTRAL) :
		color = { 1.0f, 1.0f, 1.0f };
		break;
	}
	return color;
}