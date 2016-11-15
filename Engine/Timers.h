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
	std::vector<std::pair<float, std::string>> ReadMSAll();

private:
	std::map<uint, PerfTimer> perfTimers;
	std::map<uint, Timer> stdTimers;

	std::map<std::string, uint> timerIDs;
};

#endif // !__TIMERMANAGER__
