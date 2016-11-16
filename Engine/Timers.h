#ifndef __TIMERMANAGER__
#define __TIMERMANAGER__

#include <map>
#include <vector>
#include "Globals.h"

class Timer;
class PerfTimer;

class TimerManager
{
public:
	TimerManager();
	~TimerManager();

	void CreatePerfTimer(std::string key);
	void CreateTimer(std::string key);
	void StartTimer(std::string key);
	void StartTimerPerf(std::string key);
	void ResetTimerStoredVal(std::string key);
	float ReadMsStoredVal(std::string key);
	float ReadMs(std::string key);
	float ReadMS_Max(std::string key);
	uint ReadSec(std::string key);
	std::string GetKeyFromID(uint id);
	std::vector<std::pair<std::string, float>> GetLastReads();

private:
	//Map that associates each ID with it's PerfTimer
	std::map<uint, PerfTimer> perfTimers;
	//Map that associates each ID with it's timer
	std::map<uint, Timer> stdTimers;

	//Map that associates each ID with its key and stored value
	std::map<uint, std::pair<std::string, float>> lastReads;

	//Map that associates each key with its ID
	std::map<std::string, uint> timerIDs;
};

#endif // !__TIMERMANAGER__
