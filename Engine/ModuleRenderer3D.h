#ifndef __MODULE_RENDER_3D__
#define __MODULE_RENDER_3D__

#include "Module.h"
#include "Globals.h"
#include "glmath.h"
#include "Light.h"

#define MAX_LIGHTS 8

struct Mesh_RenderInfo;

class ModuleRenderer3D : public Module
{
public:
	ModuleRenderer3D(Application* app, bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init();
	bool Start();
	update_status PreUpdate(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();

	void OnScreenResize(int width, int heigth);
	void UpdateProjectionMatrix();

	void DrawLine(float3 a, float3 b, float4 color = float4(0.1f, 0.58f, 0.2f, 1.0f));
	void DrawBox(float3* corners, float4 color = float4(1.0f, 1.0f, 0.6f, 1.0f));
	void DrawLocator(float4x4 transform = float4x4::identity, float4 color = float4(0.1f, 0.58f, 0.2f, 1.0f));
	void DrawMesh(Mesh_RenderInfo meshInfo);
private:
	void RenderMeshWired(const Mesh_RenderInfo& data);
	void RenderMeshFilled(const Mesh_RenderInfo& data);
	void RenderNormals(const Mesh_RenderInfo& data);
public:
	Light lights[MAX_LIGHTS];
private:
	SDL_GLContext context;
	mat3x3 NormalMatrix;
	mat4x4 ModelMatrix, ViewMatrix, ProjectionMatrix;
};

#endif