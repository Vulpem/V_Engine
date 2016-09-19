#include "Globals.h"
#include "Application.h"

#include "ModuleUI.h"
#include "ModuleWindow.h"



ModuleTests::ModuleTests(Application* app, bool start_enabled) : Module(app, start_enabled)
{

}

// Destructor
ModuleTests::~ModuleTests()
{

}

// Called before render is available
bool ModuleTests::Init()
{
	bool ret = true;

		
	return ret;
}

bool ModuleTests::Start()
{
	
	return true;
}

// Called every draw update
update_status ModuleTests::PreUpdate(float dt)
{
	update_status ret = UPDATE_CONTINUE;

	return ret;
}

update_status ModuleTests::Update(float dt)
{
	float3* points = new float3[16];
	for (int n = 0; n < 5; n++)
	{
		points[n].x = rand()%10 - 20;
		points[n].y = rand() % 10 - 20;
		points[n].z = rand() % 10 - 20;
	}

	AABB box1;
	box1.minPoint = float3(0, 0, 0);
	box1.maxPoint = float3(10, 10, 10);
	box1.MinimalEnclosingAABB(points, 4);


	OBB obb1 = box1.ToOBB();
	obb1.Transform(Quat(1, 1, 1, 1));

	obb1.GetCornerPoints(points);

	Sphere sphere1;

	sphere1.FastEnclosingSphere(points, 8);


	delete[] points;
	return UPDATE_CONTINUE;
}

update_status ModuleTests::PostUpdate(float dt)
{
	
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleTests::CleanUp()
{
	
	return true;
}