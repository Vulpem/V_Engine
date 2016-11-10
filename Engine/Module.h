#ifndef __MODULE__
#define __MODULE__

class Application;
struct PhysBody3D;
struct viewPort;

#include "C_String.h"
#include "PugiXml\src\pugixml.hpp"

class Module
{
private :
	bool enabled;

protected:
	C_String name;

public:
	Application* App;

	Module(Application* parent, bool start_enabled = true) : App(parent)
	{
		enabled = start_enabled;
	}

	virtual ~Module()
	{}

	bool IsEnabled() const
	{
		return enabled;
	}

	void Enable()
	{
		if (enabled == false)
		{
			enabled = true;
			Start();
		}
	}

	bool Disable()
	{
		bool ret = true;
		if (enabled == true)
		{
			enabled = false;
			ret = CleanUp();
		}
		return ret;
	}

	virtual bool Init() 
	{
		return true; 
	}

	virtual bool Start()
	{
		return true;
	}

	virtual update_status PreUpdate(float dt)
	{
		return UPDATE_CONTINUE;
	}

	virtual update_status Update(float dt)
	{
		return UPDATE_CONTINUE;
	}

	virtual void Render(const viewPort& port)
	{

	}

	virtual update_status PostUpdate(float dt)
	{
		return UPDATE_CONTINUE;
	}

	virtual bool CleanUp() 
	{ 
		enabled = false;
		return true; 
	}

	C_String GetName()
	{
		return name;
	}

	virtual void OnCollision(PhysBody3D* body1, PhysBody3D* body2)
	{}

	virtual void OnScreenResize(int width, int heigth) {  };
};

#endif