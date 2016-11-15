#include"Timers.h"

#include "PerfTimer.h"
#include "Timer.h"

TimerManager::TimerManager()
{
}

TimerManager::~TimerManager()
{
}

void TimerManager::CreatePerfTimer(std::string key)
{
	uint id = timerIDs.size();
	timerIDs.insert(std::pair<std::string, uint>(key, id));

	std::pair<uint, PerfTimer> timer(id, PerfTimer());
	perfTimers.insert(timer);
	std::map<uint, PerfTimer>::iterator it = perfTimers.find(id);
	it->second.Start();
}

void TimerManager::CreateTimer(std::string key)
{
	uint id = timerIDs.size();
	timerIDs.insert(std::pair<std::string, uint>(key, id));

	std::pair<uint, Timer> timer(id, Timer());
	stdTimers.insert(timer);
	std::map<uint, Timer>::iterator it = stdTimers.find(id);
	it->second.Start();
}

void TimerManager::StartTimer(std::string key)
{
	std::map<std::string, uint>::iterator IDit = timerIDs.find(key);
	if (IDit != timerIDs.end())
	{
		std::map<uint, PerfTimer>::iterator it = perfTimers.find(IDit->second);
		if (it != perfTimers.end())
		{
			it->second.Start();
		}
		else
		{
			std::map<uint, Timer>::iterator stdIt = stdTimers.find(IDit->second);
			if (stdIt != stdTimers.end())
			{
				stdIt->second.Start();
			}
			else
			{
				LOG("Something went horribly wrong with timers. ID %u with key %s has no timer associated", IDit->second, key.data());
			}
		}
	}
	else
	{
		CreateTimer(key);
	}
}

float TimerManager::ReadMs(std::string key)
{
	std::map<std::string, uint>::iterator IDit = timerIDs.find(key);
	if (IDit != timerIDs.end())
	{
		std::map<uint, PerfTimer>::iterator it = perfTimers.find(IDit->second);
		if (it != perfTimers.end())
		{
			return it->second.ReadMs();
		}
		else
		{
			std::map<uint, Timer>::iterator stdIt = stdTimers.find(IDit->second);
			if (stdIt != stdTimers.end())
			{
				return stdIt->second.Read();
			}
			else
			{
				LOG("Something went horribly wrong with timers. ID %u with key %s has no timer associated", IDit->second, key.data());
			}
		}
	}
	else
	{
		LOG("Tried to read the timer %s, which is unexistant", key.data());
	}
	return 0.0f;
}

uint TimerManager::ReadSec(std::string key)
{
	return ReadMs(key) / 1000;
}

std::string TimerManager::GetKeyFromID(uint id)
{
	for (std::map<std::string, uint>::iterator it = timerIDs.begin(); it != timerIDs.end(); it++)
	{
		if (it->second == id)
		{
			return it->first;
		}
	}
	return std::string();
}

std::vector<std::pair<float, std::string>> TimerManager::ReadMSAll()
{
	std::vector<std::pair<float, std::string>> ret;
	for (std::map<uint, PerfTimer>::iterator it = perfTimers.begin(); it != perfTimers.end(); it++)
	{
		ret.push_back(std::pair<float, std::string>(it->second.ReadMs(), GetKeyFromID(it->first)));
	}

	for (std::map<uint, Timer>::iterator it = stdTimers.begin(); it != stdTimers.end(); it++)
	{
		ret.push_back(std::pair<float, std::string>(it->second.Read(), GetKeyFromID(it->first)));
	}

	return ret;
}
