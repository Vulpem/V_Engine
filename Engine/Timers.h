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
	float ReadMs(std::string key);
	uint ReadSec(std::string key);
	std::string GetKeyFromID(uint id);
	std::vector<std::pair<std::string, float>> GetLastReads();

private:
	std::map<uint, PerfTimer> perfTimers;
	std::map<uint, Timer> stdTimers;

	std::map<std::string, uint> timerIDs;
	std::map<uint, std::pair<std::string, float>> lastReads;
};

#endif // !__TIMERMANAGER__
