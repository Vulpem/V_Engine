#include "Globals.h"
#include "Application.h"
#include "Timers.h"

void CreatePerfTimer(std::string key)
{
	App->timers->CreatePerfTimer(key);
}

void CreateTimer(std::string key)
{
	App->timers->CreateTimer(key);
}

float ReadMs(std::string key)
{
	return App->timers->ReadMs(key);
}

unsigned int ReadSec(std::string key)
{
	return App->timers->ReadSec(key);
}

void StartTimer(std::string key)
{
	App->timers->StartTimer(key);
}

void StartTimerPerf(std::string key)
{
	App->timers->StartTimerPerf(key);
}

float ReadMs_Max(std::string key)
{
	return App->timers->ReadMS_Max(key);
}

void ResetTimerStoredVal(std::string key)
{
	App->timers->ResetTimerStoredVal(key);
}

float ReadMsStoredVal(std::string key)
{
	return App->timers->ReadMsStoredVal(key);
}