#ifndef __GLOBALS__
#define __GLOBALS__

#include <windows.h>
#include <stdio.h>
#include <string>

#define LOG(format, ...) log(__FILE__, __LINE__, format, __VA_ARGS__);

void log(const char file[], int line, const char* format, ...);


// ------------ TIMER FUNCTIONS -------------------------

#define CREATE_PERF_TIMER(key) CreatePerfTimer(key);
void CreatePerfTimer(std::string key);

#define CREATE_TIMER(key) CreateTimer(key);
void CreateTimer(std::string key);

#define READ_MS(key) ReadMs(key);
float ReadMs(std::string key);

#define READ_SECONDS(key) ReadSec(key);
unsigned int ReadSec(std::string key);

#define START_TIMER(key) StartTimer(key);
void StartTimer(std::string key);

// ------------ TIMER FUNCTIONS -------------------------


#define CAP(n) ((n <= 0.0f) ? n=0.0f : (n >= 1.0f) ? n=1.0f : n=n)

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f
#define HAVE_M_PI

#define EDITOR_FRAME_SAMPLES 50

#define CAP(n) ((n <= 0.0f) ? n=0.0f : (n >= 1.0f) ? n=1.0f : n=n)
#define MIN(a,b) ((a)<(b)) ? (a) : (b)
#define MAX(a,b) ((a)>(b)) ? (a) : (b)

typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;
typedef unsigned int uint;

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

enum update_status
{
	UPDATE_CONTINUE = 1,
	UPDATE_STOP,
	UPDATE_ERROR
};


// Configuration -----------
#define SCREEN_WIDTH 1456
#define SCREEN_HEIGHT 1024
#define WIN_MAXIMIZED true
#define WIN_FULLSCREEN false
#define WIN_RESIZABLE true
#define WIN_BORDERLESS false
#define WIN_FULLSCREEN_DESKTOP false
#define VSYNC false

#define TITLE "V_Engine"
#define ORGANISATION "UPC"

//USED WHEN GENERATING THE QUAD TREE SIZE
#define WORLD_WIDTH 1400
#define WORLD_DEPTH 1400
#define WORLD_HEIGHT 30

#endif