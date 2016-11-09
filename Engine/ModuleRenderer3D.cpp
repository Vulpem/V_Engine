#include "Globals.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleWindow.h"
#include "ModuleCamera3D.h"

#include "Camera.h"

#include "OpenGL.h"

#include "Mesh_RenderInfo.h"
#include "ViewPort.h"

#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "Glew/libx86/glew32.lib") /* link Microsoft OpenGL lib   */




ModuleRenderer3D::ModuleRenderer3D(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name.create("ModuleRenderer3D");
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

		if (VSYNC && SDL_GL_SetSwapInterval(1) < 0)
			LOG("Warning: Unable to set VSync! SDL Error: %s", SDL_GetError());

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

	}

	return ret;
}

bool ModuleRenderer3D::Start()
{
	bool ret = true;
	//Initialize Projection Matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(App->camera->GetProjectionMatrix());

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
update_status ModuleRenderer3D::PreUpdate(float dt)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// light 0 on cam pos
	float3 camPos = App->camera->GetCamPos();
	lights[0].SetPos(camPos.x, camPos.y, camPos.z);

	return UPDATE_CONTINUE;
}

// PostUpdate present buffer to screen
update_status ModuleRenderer3D::PostUpdate(float dt)
{
	for (uint i = 0; i < MAX_LIGHTS; ++i)
	{
		lights[i].Render();
	}

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
}

void ModuleRenderer3D::UpdateProjectionMatrix(Camera* cam)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(cam->GetProjectionMatrix().ptr());
	glMatrixMode(GL_MODELVIEW);
}

void ModuleRenderer3D::DrawLine(float3 a, float3 b, float4 color)
{
	glDisable(GL_LIGHTING);
	glLineWidth(2.0f);
	glColor4f(color.x, color.y, color.z, color.w);

	glBegin(GL_LINES);

	glVertex3fv(a.ptr()); glVertex3fv(b.ptr());

	glEnd();

	glLineWidth(1.0f);
	glEnable(GL_LIGHTING);
}

void ModuleRenderer3D::DrawBox(float3* corners, float4 color)
{
	glDisable(GL_LIGHTING);
	glLineWidth(2.0f);
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

	glLineWidth(1.0f);
	glEnable(GL_LIGHTING);
}

void ModuleRenderer3D::DrawLocator(float4x4 transform, float4 color)
{
	glPushMatrix();
	glMultMatrixf(transform.ptr());
	glDisable(GL_LIGHTING);
	// Draw Axis Grid
	glLineWidth(2.0f);

	glBegin(GL_LINES);

	glColor4f(color.x, color.y, color.z, color.w);

	glVertex3f(1.0f, 0.0f, 0.0f); glVertex3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f); glVertex3f(0.0f, -1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 1.0f); glVertex3f(0.0f, 0.0f, -1.0f);
	//Arrow indicating forward
	glVertex3f(0.0f, 0.0f, 1.0f); glVertex3f(0.1f, 0.0f, 0.9f);
	glVertex3f(0.0f, 0.0f, 1.0f); glVertex3f(-0.1f, 0.0f, 0.9f);

	glEnd();

	glLineWidth(1.0f);
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

void ModuleRenderer3D::DrawMesh(Mesh_RenderInfo meshInfo)
{
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

	if (meshInfo.textureCoordsBuffer > 0)
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		//Setting texture coords
		glBindBuffer(GL_ARRAY_BUFFER, meshInfo.textureCoordsBuffer);
		glTexCoordPointer(2, GL_FLOAT, 0, NULL);

		glBindTexture(GL_TEXTURE_2D, meshInfo.textureBuffer);
	}
	if (meshInfo.normalsBuffer > 0)
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		//Setting Normals
		glBindBuffer(GL_ARRAY_BUFFER, meshInfo.normalsBuffer);
		glNormalPointer(GL_FLOAT, 0, NULL);
	}

	//Setting index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshInfo.indicesBuffer);
	if (meshInfo.wired)
	{
		RenderMeshWired(meshInfo);
	}
	if (meshInfo.filled)
	{
		RenderMeshFilled(meshInfo);
	}

	glDrawElements(GL_TRIANGLES, meshInfo.num_indices, GL_UNSIGNED_INT, NULL);

	//Cleaning
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glPopMatrix();
}

void ModuleRenderer3D::SetViewPort(viewPort port)
{
	port.SetCameraMatrix();
	glViewport(port.pos.x, App->window->GetWindowSize().y - (port.size.y + port.pos.y), port.size.x, port.size.y);
	UpdateProjectionMatrix(port.camera);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(port.camera->GetViewMatrix().ptr());
}

void ModuleRenderer3D::RenderMeshWired(const Mesh_RenderInfo& data)
{
		glDisable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(1.0f);
		glColor4fv(data.wiresColor.ptr());
		if (data.doubleSidedFaces)
		{
			glDisable(GL_CULL_FACE);
		}

		glDrawElements(GL_TRIANGLES, data.num_indices, GL_UNSIGNED_INT, NULL);

		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void ModuleRenderer3D::RenderMeshFilled(const Mesh_RenderInfo& data)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
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
