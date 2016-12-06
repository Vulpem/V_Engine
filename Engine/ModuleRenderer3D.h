#ifndef __MODULE_RENDER_3D__
#define __MODULE_RENDER_3D__

#include "Module.h"
#include "Globals.h"
#include "Light.h"
#include <map>

#define MAX_LIGHTS 8

struct Mesh_RenderInfo;
struct viewPort;
class Camera;

class ModuleRenderer3D : public Module
{
public:
	ModuleRenderer3D(Application* app, bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init();
	bool Start();
	update_status PreUpdate();
	update_status PostUpdate();
	bool CleanUp();

	void OnScreenResize(int width, int heigth);
	void UpdateProjectionMatrix(Camera* cam);

	void RenderBlendObjects();

	void DrawLine(float3 a, float3 b, float4 color = float4(0.1f, 0.58f, 0.2f, 1.0f));
	void DrawBox(float3* corners, float4 color = float4(1.0f, 1.0f, 0.6f, 1.0f));
	void DrawLocator(float4x4 transform = float4x4::identity, float4 color = float4(0.1f, 0.58f, 0.2f, 1.0f));
	void DrawLocator(float3 position, float4 color = float4(0.1f, 0.58f, 0.2f, 1.0f));
	void DrawMesh(Mesh_RenderInfo& meshInfo, bool renderBlends = false);

	viewPort* HoveringViewPort();
	float2 ViewPortToScreen(const float2& pos_in_ViewPort, viewPort** OUT_port = NULL);
	float2 ScreenToViewPort(const float2& pos_in_screen, viewPort** OUT_port = NULL);

	uint AddViewPort(float2 pos, float2 size, Camera* cam);
	viewPort* FindViewPort(uint ID);
	bool DeleteViewPort(uint ID);

	void SetViewPort(viewPort& port);
private:
	void RenderMeshWired(const Mesh_RenderInfo& data);
	void RenderMeshFilled(const Mesh_RenderInfo& data);
	void RenderNormals(const Mesh_RenderInfo& data);
public:
	std::vector<viewPort> viewPorts;
	Light lights[MAX_LIGHTS];

private:
	std::multimap<float, Mesh_RenderInfo> alphaObjects;
	viewPort* currentViewPort = nullptr;

	SDL_GLContext context;

	//Only for read access
	bool usingLights = true;
	//Only for read access
	bool usingSingleSidedFaces = true;
	//Only for read access
	bool usingTextures = true;
};

#endif