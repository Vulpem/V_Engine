#pragma once
#include "Globals.h"

class mesh
{
public:
	mesh();
	~mesh();

	char* name;

	uint id_vertices = 0;
	uint num_vertices = 0;
	float* vertices = nullptr;

	uint id_indices = 0;
	uint num_indices = 0;
	uint* indices = nullptr;

	uint id_normals = 0;
	uint num_normals;
	float* normals = nullptr;

	uint id_textureCoords = 0;
	uint num_textureCoords = 0;
	float* textureCoords = nullptr;


	float r = 0.5f;
	float g = 0.5f;
	float b = 0.5f;
	float a = 1.0f;

	uint texture = 0;

	bool wires = false;
	bool selected = false;

	void Draw();
private:
	void RealRender(bool wired = false);
};