#include "Globals.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleWindow.h"
#include "ModuleCamera3D.h"

#include "OpenGL.h"

#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "Glew/libx86/glew32.lib") /* link Microsoft OpenGL lib   */




ModuleRenderer3D::ModuleRenderer3D(Application* app, bool start_enabled) : Module(app, start_enabled)
{
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

		//Initialize Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();


		ProjectionMatrix = perspective(60.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.125f, 512.0f);
		glLoadMatrixf(&ProjectionMatrix);


		//Check for error
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		//Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			LOG("Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glClearDepth(10.0f);

		//Initialize clear color
		glClearColor(0.f, 0.f, 0.f, 1.f);

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
		lights[0].ambient.Set(0.25f, 0.25f, 0.25f, 1.0f);
		lights[0].diffuse.Set(0.75f, 0.75f, 0.75f, 1.0f);
		lights[0].SetPos(0.0f, 0.0f, 2.5f);
		lights[0].Init();

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

		
//TMP
#pragma region VertexArrayMode
		glGenBuffers(1, (GLuint*) &(vertexArrayBuffer));


		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		//Back
		vertexs[0] = 1;
		vertexs[1] = 1;
		vertexs[2] = -1;

		vertexs[3] = -1;
		vertexs[4] = -1;
		vertexs[5] = -1;

		vertexs[6] = -1;
		vertexs[7] = 1;
		vertexs[8] = -1;
	

		vertexs[9] = 1;
		vertexs[10] = 1;
		vertexs[11] = -1;

		vertexs[12] = 1;
		vertexs[13] = -1;
		vertexs[14] = -1;

		vertexs[15] = -1;
		vertexs[16] = -1;
		vertexs[17] = -1;

		//Left
		vertexs[18] = 1;
		vertexs[19] = 1;
		vertexs[20] = -1;

		vertexs[21] = 1;
		vertexs[22] = -1;
		vertexs[23] = 1;

		vertexs[24] = 1;
		vertexs[25] = -1;
		vertexs[26] = -1;
	

		vertexs[27] = 1;
		vertexs[28] = 1;
		vertexs[29] = -1;

		vertexs[30] = 1;
		vertexs[31] = 1;
		vertexs[32] = 1;

		vertexs[33] = 1;
		vertexs[34] = -1;
		vertexs[35] = 1;

		//Front
		vertexs[36] = 1;
		vertexs[37] = 1;
		vertexs[38] = 1;

		vertexs[39] = -1;
		vertexs[40] = 1;
		vertexs[41] = 1;

		vertexs[42] = 1;
		vertexs[43] = -1;
		vertexs[44] = 1;


		vertexs[45] = -1;
		vertexs[46] = 1;
		vertexs[47] = 1;

		vertexs[48] = -1;
		vertexs[49] = -1;
		vertexs[50] = 1;

		vertexs[51] = 1;
		vertexs[52] = -1;
		vertexs[53] = 1;

		//Right
		vertexs[54] = -1;
		vertexs[55] = 1;
		vertexs[56] = 1;

		vertexs[57] = -1;
		vertexs[58] = -1;
		vertexs[59] = -1;

		vertexs[60] = -1;
		vertexs[61] = -1;
		vertexs[62] = 1;


		vertexs[63] = -1;
		vertexs[64] = 1;
		vertexs[65] = 1;

		vertexs[66] = -1;
		vertexs[67] = 1;
		vertexs[68] = -1;

		vertexs[69] = -1;
		vertexs[70] = -1;
		vertexs[71] = -1;

		//Top
		vertexs[72] = 1;
		vertexs[73] = 1;
		vertexs[74] = 1;

		vertexs[75] = 1;
		vertexs[76] = 1;
		vertexs[77] = -1;

		vertexs[78] = -1;
		vertexs[79] = 1;
		vertexs[80] = 1;


		vertexs[81] = -1;
		vertexs[82] = 1;
		vertexs[83] = 1;

		vertexs[84] = 1;
		vertexs[85] = 1;
		vertexs[86] = -1;

		vertexs[87] = -1;
		vertexs[88] = 1;
		vertexs[89] = -1;

		//Down
		vertexs[90] = 1;
		vertexs[91] = -1;
		vertexs[92] = 1;

		vertexs[93] = -1;
		vertexs[94] = -1;
		vertexs[95] = 1;

		vertexs[96] = 1;
		vertexs[97] = -1;
		vertexs[98] = -1;


		vertexs[99] = -1;
		vertexs[100] = -1;
		vertexs[101] = 1;

		vertexs[102] = -1;
		vertexs[103] = -1;
		vertexs[104] = -1;

		vertexs[105] = 1;
		vertexs[106] = -1;
		vertexs[107] = -1;

		glBindBuffer(GL_ARRAY_BUFFER, vertexArrayBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 36 * 3, vertexs, GL_STATIC_DRAW);
#pragma endregion


#pragma region Indexed Mode

		glGenBuffers(1, (GLuint*) &(indexArrayBuffer));
		glGenBuffers(1, (GLuint*) &(indexVertexArrayBuffer));

		indexVertex[0] = 1;
		indexVertex[1] = 1;
		indexVertex[2] = 1;

		indexVertex[3] = 1;
		indexVertex[4] = 1;
		indexVertex[5] = -1;

		indexVertex[6] = -1;
		indexVertex[7] = 1;
		indexVertex[8] = -1;

		indexVertex[9] = -1;
		indexVertex[10] = 1;
		indexVertex[11] = 1;


		indexVertex[12] = 1;
		indexVertex[13] = -1;
		indexVertex[14] = 1;

		indexVertex[15] = 1;
		indexVertex[16] = -1;
		indexVertex[17] = -1;

		indexVertex[18] = -1;
		indexVertex[19] = -1;
		indexVertex[20] = -1;

		indexVertex[21] = -1;
		indexVertex[22] = -1;
		indexVertex[23] = 1;

		for (int n = 0; n < 24; n += 3)
		{
			indexVertex[n] -= 5;
		}

		glBindBuffer(GL_ARRAY_BUFFER, indexVertexArrayBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * 3, indexVertex, GL_STATIC_DRAW);

		//Top
		indexArray[0] = 0;
		indexArray[1] = 1;
		indexArray[2] = 2;

		indexArray[3] = 0;
		indexArray[4] = 2;
		indexArray[5] = 3;

		//Bottom
		indexArray[6] = 4;
		indexArray[7] = 6;
		indexArray[8] = 5;

		indexArray[9] = 4;
		indexArray[10] = 7;
		indexArray[11] = 6;

		//Left
		indexArray[12] = 0;
		indexArray[13] = 4;
		indexArray[14] = 5;

		indexArray[15] = 0;
		indexArray[16] = 5;
		indexArray[17] = 1;

		//Front
		indexArray[18] = 0;
		indexArray[19] = 3;
		indexArray[20] = 7;

		indexArray[21] = 0;
		indexArray[22] = 7;
		indexArray[23] = 4;

		//Right
		indexArray[24] = 7;
		indexArray[25] = 3;
		indexArray[26] = 2;

		indexArray[27] = 7;
		indexArray[28] = 2;
		indexArray[29] = 6;

		//Back
		indexArray[30] = 2;
		indexArray[31] = 1;
		indexArray[32] = 5;

		indexArray[33] = 2;
		indexArray[34] = 5;
		indexArray[35] = 6;

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexArrayBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * 36, indexArray, GL_STATIC_DRAW);

#pragma endregion

	}

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate(float dt)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(App->camera->GetViewMatrix());

	// light 0 on cam pos
	lights[0].SetPos(App->camera->Position.x, App->camera->Position.y, App->camera->Position.z);

	for(uint i = 0; i < MAX_LIGHTS; ++i)
		lights[i].Render();


	//Vertex Buffer draw
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, vertexArrayBuffer);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glDrawArrays(GL_TRIANGLES, 0, 36 * 3);
	glDisableClientState(GL_VERTEX_ARRAY);
	
	//Endof - VErtex Buffer draw

	//Index Buffer draw

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, indexVertexArrayBuffer);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexArrayBuffer);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);

	glDisableClientState(GL_VERTEX_ARRAY);

	while (glGetError() != GL_NO_ERROR)
	{
		int a = 0;
	}

	//Index - VErtex Buffer draw


	return UPDATE_CONTINUE;
}

// PostUpdate present buffer to screen
update_status ModuleRenderer3D::PostUpdate(float dt)
{
	SDL_GL_SwapWindow(App->window->window);
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleRenderer3D::CleanUp()
{
	LOG("Destroying 3D Renderer");

	SDL_GL_DeleteContext(context);

	return true;
}


void ModuleRenderer3D::OnResize(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	ProjectionMatrix = perspective(60.0f, (float)width / (float)height, 0.125f, 512.0f);
	glLoadMatrixf(&ProjectionMatrix);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
