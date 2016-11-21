#ifndef __GLOBALTIME__
#define __GLOBALTIME__

struct GlobalTime
{
	//Time since the application started
	long float runningTime = 0.0f;

	//Delta time from last frame in seconds
	float dt = 0.001f;

	//Game delta time from last frame in seconds
	float gdt = 0.001f;

	//Is the game in "Play mode"?
	bool playing = false;

};

#endif // !__GLOBALTIME__

