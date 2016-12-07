#include "Globals.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleWindow.h"
#include "ModuleCamera3D.h"
#include "ModuleInput.h"

#include "GameObject.h"
#include "Transform.h"

#include "Camera.h"
#include "imGUI\imgui.h"

#include "OpenGL.h"

#include "Mesh_RenderInfo.h"
#include "ViewPort.h"

#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "Glew/libx86/glew32.lib") /* link Microsoft OpenGL lib   */


ModuleRenderer3D::ModuleRenderer3D(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	moduleName = "ModuleRenderer3D";
}

// Destructor
ModuleRenderer3D::~ModuleRenderer3D()
{}

// Called before render is available
bool ModuleRenderer3D::Init()
{
	LOG("Creating 3D Renderer context");
	bool ret = true;

	//Create context
	context = SDL_GL_CreateContext(App->window->GetWindow());
	if (context == nullptr)
	{
		LOG("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	GLenum err = glewInit();

	if (err != GLEW_OK)
	{
		LOG("Glew library could not init %s\n", glewGetErrorString(err));
		ret = false;
	}
	else
		LOG("Using Glew %s", glewGetString(GLEW_VERSION));

	if (ret == true)
	{
		// get version info
		LOG("Vendor: %s", glGetString(GL_VENDOR));
		LOG("Renderer: %s", glGetString(GL_RENDERER));
		LOG("OpenGL version supported %s", glGetString(GL_VERSION));
		LOG("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

		if (VSYNC)
		{
			if (SDL_GL_SetSwapInterval(1) < 0)
			{
				LOG("Warning: Unable to set VSync! SDL Error: %s", SDL_GetError());
			}
		}
		else
		{
			if (SDL_GL_SetSwapInterval(0) < 0)
			{
				LOG("Warning: Unable to set VSync! SDL Error: %s", SDL_GetError());
			}
		}

		//Check for error
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glClearDepth(10.0f);

		//Initialize clear color
		glClearColor(0.78f, 0.81f, 0.84f, 1.f);

		//Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		// Blend for transparency
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		GLfloat LightModelAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);

		lights[0].ref = GL_LIGHT0;
		lights[1].ref = GL_LIGHT1;
		lights[2].ref = GL_LIGHT2;
		lights[3].ref = GL_LIGHT3;
		lights[4].ref = GL_LIGHT4;
		lights[5].ref = GL_LIGHT5;
		lights[6].ref = GL_LIGHT6;
		lights[7].ref = GL_LIGHT7;
		for (int n = 0; n < MAX_LIGHTS; n++)
		{
			lights[n].ambient.Set(0.25f, 0.25f, 0.25f, 1.0f);
			lights[n].diffuse.Set(0.75f, 0.75f, 0.75f, 1.0f);
			lights[n].SetPos(n * 5, 10, 0);
			lights[n].Init();
		}

		GLfloat MaterialAmbient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

		GLfloat MaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_TEXTURE_2D);

		lights[0].Active(true);

		glShadeModel(GL_SMOOTH);		 // Enables Smooth Shading

		glLineWidth(1.0f);

		viewPorts.push_back(viewPort(float2(0, 0), float2(SCREEN_WIDTH, SCREEN_HEIGHT), App->camera->GetDefaultCam(), viewPorts.size()));

	}

	return ret;
}

bool ModuleRenderer3D::Start()
{
	bool ret = true;
	//Initialize Projection Matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//Initialize Modelview Matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//Check for error
	
	while (GLenum error = glGetError() != GL_NO_ERROR)
	{
		LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
		ret = false;
	}

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	return UPDATE_CONTINUE;
}

// PostUpdate present buffer to screen
update_status ModuleRenderer3D::PostUpdate()
{
	for (uint i = 0; i < MAX_LIGHTS; ++i)
	{
		lights[i].Render();
	}

	viewPorts.front().camera = App->camera->GetDefaultCam();
	TIMER_START("ViewPorts render");
	if (viewPorts.empty() == false)
	{
		TIMER_RESET_STORED("ViewPorts slowest");
		for (std::vector<viewPort>::iterator port = viewPorts.begin(); port != viewPorts.end(); port++)
		{
			TIMER_START_PERF("ViewPorts slowest");
			if (port->active && port->autoRender)
			{
				SetViewPort(*port);
				App->Render(*port);
				RenderBlendObjects();
			}
			TIMER_READ_MS_MAX("ViewPorts slowest");
		}
	}
	else
	{
		LOG("Warning, there are no viewPorts!");
	}
	TIMER_READ_MS("ViewPorts render");

#if USE_EDITOR
	ImGui::Render();
#endif

	SDL_GL_SwapWindow(App->window->GetWindow());
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleRenderer3D::CleanUp()
{
	LOG("Destroying 3D Renderer");

	SDL_GL_DeleteContext(context);

	return true;
}


void ModuleRenderer3D::OnScreenResize(int width, int heigth)
{
	viewPorts.front().size = float2(width, heigth);
}

void ModuleRenderer3D::UpdateProjectionMatrix(Camera* cam)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(cam->GetProjectionMatrix().ptr());
	glMatrixMode(GL_MODELVIEW);
}

void ModuleRenderer3D::RenderBlendObjects()
{
	std::multimap<float, Mesh_RenderInfo>::reverse_iterator it = alphaObjects.rbegin();
	for (; it != alphaObjects.rend(); it++)
	{
		DrawMesh(it->second, true);
	}
	alphaObjects.clear();
}

void ModuleRenderer3D::DrawLine(float3 a, float3 b, float4 color)
{
	if (usingLights)
	{
		glDisable(GL_LIGHTING);
	}
	glColor4f(color.x, color.y, color.z, color.w);

	glBegin(GL_LINES);

	glVertex3fv(a.ptr()); glVertex3fv(b.ptr());

	glEnd();

	if (usingLights)
	{
		glEnable(GL_LIGHTING);
	}
}

void ModuleRenderer3D::DrawBox(float3* corners, float4 color)
{
	if (usingLights)
	{
		glDisable(GL_LIGHTING);
	}
	glColor4f(color.x, color.y, color.z, color.w);

	glBegin(GL_LINES);

	glVertex3fv(corners[0].ptr()); glVertex3fv(corners[1].ptr());
	glVertex3fv(corners[0].ptr()); glVertex3fv(corners[2].ptr());
	glVertex3fv(corners[0].ptr()); glVertex3fv(corners[4].ptr());
	glVertex3fv(corners[3].ptr()); glVertex3fv(corners[1].ptr());
	glVertex3fv(corners[3].ptr()); glVertex3fv(corners[2].ptr());
	glVertex3fv(corners[3].ptr()); glVertex3fv(corners[7].ptr());
	glVertex3fv(corners[5].ptr()); glVertex3fv(corners[1].ptr());
	glVertex3fv(corners[5].ptr()); glVertex3fv(corners[4].ptr());
	glVertex3fv(corners[5].ptr()); glVertex3fv(corners[7].ptr());
	glVertex3fv(corners[6].ptr()); glVertex3fv(corners[2].ptr());
	glVertex3fv(corners[6].ptr()); glVertex3fv(corners[4].ptr());
	glVertex3fv(corners[6].ptr()); glVertex3fv(corners[7].ptr());

	glEnd();

	if (usingLights)
	{
		glEnable(GL_LIGHTING);
	}
}

void ModuleRenderer3D::DrawLocator(float4x4 transform, float4 color)
{
	if (usingLights)
	{
		glDisable(GL_LIGHTING);
	}
	glPushMatrix();
	glMultMatrixf(transform.ptr());

	glColor4f(color.x, color.y, color.z, color.w);

	glBegin(GL_LINES);

	glVertex3f(1.0f, 0.0f, 0.0f); glVertex3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f); glVertex3f(0.0f, -1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 1.0f); glVertex3f(0.0f, 0.0f, -1.0f);
	//Arrow indicating forward
	glVertex3f(0.0f, 0.0f, 1.0f); glVertex3f(0.1f, 0.0f, 0.9f);
	glVertex3f(0.0f, 0.0f, 1.0f); glVertex3f(-0.1f, 0.0f, 0.9f);

	glEnd();

	if (usingLights)
	{
		glEnable(GL_LIGHTING);
	}
	glPopMatrix();
}

void ModuleRenderer3D::DrawLocator(float3 position, float4 color)
{
	App->renderer3D->DrawLocator((float4x4::FromTRS(position, float4x4::identity, float3(1, 1, 1))).Transposed(), color);
}

void ModuleRenderer3D::DrawMesh(Mesh_RenderInfo& meshInfo, bool renderBlends)
{
	if (meshInfo.alphaType == AlphaTestTypes::ALPHA_BLEND && renderBlends == false)
	{
		//TMP / TODO
		//This is pretty inaccurate and probably not optimized. But, hey, it works. Sometimes. Maybe.
		float3 objectPos = meshInfo.transform.Transposed().TranslatePart();
		float distanceToObject = currentViewPort->camera->object->GetTransform()->GetGlobalPos().Distance(objectPos);

		alphaObjects.insert(std::pair<float, Mesh_RenderInfo>(distanceToObject, meshInfo));
		return;
	}
	glPushMatrix();
	glMultMatrixf(meshInfo.transform.ptr());

	if (meshInfo.renderNormals)
	{
		RenderNormals(meshInfo);
	}

	glEnableClientState(GL_VERTEX_ARRAY);

	//Setting vertex
	glBindBuffer(GL_ARRAY_BUFFER, meshInfo.vertexBuffer);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	//Setting index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshInfo.indicesBuffer);

	if (meshInfo.normalsBuffer > 0)
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		//Setting Normals
		glBindBuffer(GL_ARRAY_BUFFER, meshInfo.normalsBuffer);
		glNormalPointer(GL_FLOAT, 0, NULL);
	}

	if (meshInfo.wired)
	{
		RenderMeshWired(meshInfo);
	}

	//Setting alpha&&blend
	switch (meshInfo.alphaType)
	{
	case (AlphaTestTypes::ALPHA_BLEND):
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, meshInfo.blendType);
	}
	case (AlphaTestTypes::ALPHA_DISCARD):
	{
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, meshInfo.alphaTest);
		break;
	}
	}

	if (meshInfo.textureCoordsBuffer > 0)
	{
		if (meshInfo.textureBuffer > 0)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			//Setting texture coords
			glBindBuffer(GL_ARRAY_BUFFER, meshInfo.textureCoordsBuffer);
			glTexCoordPointer(2, GL_FLOAT, 0, NULL);
			glBindTexture(GL_TEXTURE_2D, meshInfo.textureBuffer);
		}
	}

	if (meshInfo.filled)
	{
		RenderMeshFilled(meshInfo);
	}

	//Cleaning
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);

	glPopMatrix();
}

viewPort* ModuleRenderer3D::HoveringViewPort()
{
	for (std::vector<viewPort>::reverse_iterator it = viewPorts.rbegin(); it != viewPorts.rend(); it++)
	{
		if (it->active)
		{
			if (App->input->GetMouseX() > it->pos.x && App->input->GetMouseX() < it->pos.x + it->size.x &&
				App->input->GetMouseY() > it->pos.y && App->input->GetMouseY() < it->pos.y + it->size.y)
			{
				return &*it;
			}
		}
	}
	return nullptr;
}

float2 ModuleRenderer3D::ViewPortToScreen(const float2 & pos_in_ViewPort, viewPort** OUT_port)
{
	*OUT_port = HoveringViewPort();
	if (*OUT_port != nullptr)
	{
		return float2((*OUT_port)->pos + pos_in_ViewPort);
	}
	return float2(-1,-1);
}

float2 ModuleRenderer3D::ScreenToViewPort(const float2 & pos_in_screen, viewPort** OUT_port)
{
	*OUT_port = HoveringViewPort();
	if (*OUT_port != nullptr)
	{
		return float2(pos_in_screen - (*OUT_port)->pos);
	}
	return float2(-1,-1);
}

uint ModuleRenderer3D::AddViewPort(float2 pos, float2 size, Camera * cam)
{
	viewPorts.push_back(viewPort(pos, size, cam, viewPorts.size()));
	return viewPorts.back().ID;
}

viewPort * ModuleRenderer3D::FindViewPort(uint ID)
{
	for (std::vector<viewPort>::iterator it = viewPorts.begin(); it != viewPorts.end(); it++)
	{
		if (it->ID == ID)
		{
			return it._Ptr;
		}
	}
	return nullptr;
}

bool ModuleRenderer3D::DeleteViewPort(uint ID)
{
	for (std::vector<viewPort>::iterator it = viewPorts.begin(); it != viewPorts.end(); it++)
	{
		if (it->ID == ID)
		{
			viewPorts.erase(it);
			return true;
		}
	}
	return false;
}

void ModuleRenderer3D::SetViewPort(viewPort& port)
{
	currentViewPort = &port;
	port.SetCameraMatrix();
	glViewport(port.pos.x, App->window->GetWindowSize().y - (port.size.y + port.pos.y), port.size.x, port.size.y);
	UpdateProjectionMatrix(port.camera);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(port.camera->GetViewMatrix().ptr());

	if (port.useSingleSidedFaces) { glEnable(GL_CULL_FACE); usingSingleSidedFaces = true; }
	else { glDisable(GL_CULL_FACE); usingSingleSidedFaces = false; }

	if (port.useLighting) { glEnable(GL_LIGHTING); usingLights = true; }
	else { glDisable(GL_LIGHTING); usingLights = false;}

	if (port.useMaterials) { glEnable(GL_TEXTURE_2D); usingTextures = true; }
	else { glDisable(GL_TEXTURE_2D); usingTextures = false;}
}

void ModuleRenderer3D::RenderMeshWired(const Mesh_RenderInfo& data)
{
	if (usingSingleSidedFaces)
	{
		glDisable(GL_CULL_FACE);
	}
	if (usingLights)
	{
		glDisable(GL_LIGHTING);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor4fv(data.wiresColor.ptr());

	glDrawElements(GL_TRIANGLES, data.num_indices, GL_UNSIGNED_INT, NULL);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (usingSingleSidedFaces)
	{
		glEnable(GL_CULL_FACE);
	}
	if (usingLights)
	{
		glEnable(GL_LIGHTING);
	}

}

void ModuleRenderer3D::RenderMeshFilled(const Mesh_RenderInfo& data)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor4fv(data.meshColor.ptr());
	glDrawElements(GL_TRIANGLES, data.num_indices, GL_UNSIGNED_INT, NULL);
}

void ModuleRenderer3D::RenderNormals(const Mesh_RenderInfo & data)
{
	if (data.normalsBuffer > 0)
	{
		float* normals = new float[data.num_vertices * 3];
		glBindBuffer(GL_ARRAY_BUFFER, data.normalsBuffer);
		glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * data.num_vertices * 3, normals);

		float* vertices = new float[data.num_vertices * 3];
		glBindBuffer(GL_ARRAY_BUFFER, data.vertexBuffer);
		glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * data.num_vertices * 3, vertices);

		for (uint n = 0; n < data.num_vertices; n++)
		{
			DrawLine(
				float3(vertices[n * 3], vertices[n * 3 + 1], vertices[n * 3 + 2]),
				float3(vertices[n * 3] + normals[n * 3], vertices[n * 3 + 1] + normals[n * 3 + 1], vertices[n * 3 + 2] + normals[n * 3 + 2]),
				float4(0.54f, 0.0f, 0.54f, 1.0f));
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		RELEASE_ARRAY(normals);
		RELEASE_ARRAY(vertices);
	}
}
