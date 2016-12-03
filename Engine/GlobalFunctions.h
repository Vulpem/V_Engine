#ifndef __GLOBALFUNCTIONS__
#define __GLOBALFUNCTIONS__

#include <windows.h>
#include <stdio.h>
#include <string>
#include <stdint.h>



#define CAP(n) ((n <= 0.0f) ? n=0.0f : (n >= 1.0f) ? n=1.0f : n=n)
#define MIN(a,b) ((a)<(b)) ? (a) : (b)
#define MAX(a,b) ((a)>(b)) ? (a) : (b)


#define RELEASE( x )\
    {\
       if( x != nullptr )\
       {\
         delete x;\
	     x = nullptr;\
       }\
    }

#define RELEASE_ARRAY( x )\
	{\
       if( x != nullptr )\
       {\
           delete[] x;\
	       x = nullptr;\
		 }\
	 }



struct GlobalTime
{
	//Time since the application started
	long float AppRuntime = 0.0f;

	//Time since the game started
	long float GameRuntime = 0.0f;

	//Delta time from last frame in seconds
	float dt = 0.001f;

	//Game delta time from last frame in seconds
	float gdt = 0.001f;

	//Game speed multiplier
	float gdtModifier = 1.0f;

	//Is the game in "Play mode"?
	bool PlayMode = false;

	//Is the game paused?
	bool Pause = false;

};


uint64_t GenerateUUID();



// ------------ TIMER FUNCTIONS -------------------------

//Each time a timer is "read" it will store it's last returned value

//Create a slower but more accurate timer associated with the key sent
#define TIMER_CREATE_PERF(key) CreatePerfTimer(key)
void CreatePerfTimer(std::string key);

//Create a timer associated with the key sent
#define TIMER_CREATE(key) CreateTimer(key)
void CreateTimer(std::string key);


//Sets the timer to 0, faster for normal timers. If it doesn't exist, it will create a timer
#define TIMER_START(key) StartTimer(key)
void StartTimer(std::string key);

//Sets the timer to 0, faster for perf timers. If it doesn't exist, it will create a Perf timer
#define TIMER_START_PERF(key) StartTimerPerf(key)
void StartTimerPerf(std::string key);

//Sets the timer stored value to 0.0f
#define TIMER_RESET_STORED(key) ResetTimerStoredVal(key)
void ResetTimerStoredVal(std::string key);


//Returns the timer value in MS, and stores the result
#define TIMER_READ_MS(key) ReadMs(key)
float ReadMs(std::string key);

//Returns the timer value in seconds, and stores the result
#define TIMER_READ_SECONDS(key) ReadSec(key)
unsigned int ReadSec(std::string key);

//Returns the higher value between the last read the timer had and the current read and stores it
#define TIMER_READ_MS_MAX(key) ReadMs_Max(key)
float ReadMs_Max(std::string key);

//Returns the timer stored value (last read)
#define TIMER_READ_STORED(key) ReadMsStoredVal(key)
float ReadMsStoredVal(std::string key);

// ------------ TIMER FUNCTIONS -------------------------

#endif // !__GLOBALTIME__

