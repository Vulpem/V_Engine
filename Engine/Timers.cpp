#include"Timers.h"

#include "PerfTimer.h"
#include "Timer.h"

TimerManager::TimerManager()
{
}

TimerManager::~TimerManager()
{
	perfTimers.clear();
	stdTimers.clear();
	lastReads.clear();
	timerIDs.clear();
}

void TimerManager::CreatePerfTimer(std::string key)
{
	uint id = timerIDs.size();
	timerIDs.insert(std::pair<std::string, uint>(key, id));

	std::pair<uint, PerfTimer> timer(id, PerfTimer());
	perfTimers.insert(timer);
	std::map<uint, PerfTimer>::iterator it = perfTimers.find(id);
	it->second.Start();

	//map with ID and a pair(key, lastReadTime)
	lastReads.insert(std::pair<uint, std::pair<std::string, float>>(id, std::pair<std::string, float>(key, 0.0f)));
}

void TimerManager::CreateTimer(std::string key)
{
	uint id = timerIDs.size();
	timerIDs.insert(std::pair<std::string, uint>(key, id));

	std::pair<uint, Timer> timer(id, Timer());
	stdTimers.insert(timer);
	std::map<uint, Timer>::iterator it = stdTimers.find(id);
	it->second.Start();

	//map with ID and a pair(key, lastReadTime)
	lastReads.insert(std::pair<uint, std::pair<std::string, float>>(id, std::pair<std::string, float>(key, 0.0f)));
}

void TimerManager::StartTimer(std::string key)
{
	std::map<std::string, uint>::iterator IDit = timerIDs.find(key);
	if (IDit != timerIDs.end())
	{
		std::map<uint, Timer>::iterator it = stdTimers.find(IDit->second);
		if (it != stdTimers.end())
		{
			it->second.Start();
		}
		else
		{
			std::map<uint, PerfTimer>::iterator stdIt = perfTimers.find(IDit->second);
			if (stdIt != perfTimers.end())
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

void TimerManager::StartTimerPerf(std::string key)
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
		CreatePerfTimer(key);
	}
}


void TimerManager::ResetTimerStoredVal(std::string key)
{
	std::map<std::string, uint>::iterator IDit = timerIDs.find(key);
	if (IDit != timerIDs.end())
	{
		lastReads.find(IDit->second)->second.second = 0.0f;
	}
}

float TimerManager::ReadMsStoredVal(std::string key)
{
	std::map<std::string, uint>::iterator IDit = timerIDs.find(key);
	if (IDit != timerIDs.end())
	{
		return lastReads.find(IDit->second)->second.second;
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
			float ret = it->second.ReadMs();
			lastReads.find(it->first)->second.second = ret;
			return ret;
		}
		else
		{
			std::map<uint, Timer>::iterator stdIt = stdTimers.find(IDit->second);
			if (stdIt != stdTimers.end())
			{
				float ret = stdIt->second.Read();
				lastReads.find(stdIt->first)->second.second = ret;
				return ret;
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

float TimerManager::ReadMS_Max(std::string key)
{
	std::map<std::string, uint>::iterator IDit = timerIDs.find(key);
	if (IDit != timerIDs.end())
	{
		std::map<uint, PerfTimer>::iterator it = perfTimers.find(IDit->second);
		if (it != perfTimers.end())
		{
			float ret = it->second.ReadMs();
			std::map<uint, std::pair<std::string, float>>::iterator tim = lastReads.find(it->first);

			ret = MAX(ret, tim->second.second);
			tim->second.second = ret;
			return ret;
		}
		else
		{
			std::map<uint, Timer>::iterator stdIt = stdTimers.find(IDit->second);
			if (stdIt != stdTimers.end())
			{
				float ret = stdIt->second.Read();
				std::map<uint, std::pair<std::string, float>>::iterator tim = lastReads.find(stdIt->first);

				ret = MAX(ret, tim->second.second);
				tim->second.second = ret;
				return ret;
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

std::vector<std::pair<std::string, float>> TimerManager::GetLastReads()
{
	std::vector<std::pair<std::string, float>> ret;
	if (lastReads.empty() == false)
	{
		std::map<std::string, uint>::iterator it = timerIDs.begin();
		for (; it != timerIDs.end(); it++)
		{
			ret.push_back(lastReads.find(it->second)->second);
		}


		//THIS WAY IS A LITTLE BIT FASTER; BUT RETURNS THE TIMERS IN THE ORDER THEY WERE CREATED
		//THE WAY CURRENTLY IMPLEMENTED RETURNS THE TIMERS IN ALPHABETICAL ORDER

		/*std::map<uint, std::pair<std::string, float>>::iterator it = lastReads.begin();
		for (; it != lastReads.end(); it++)
		{
			ret.push_back(it->second);
		}*/
	}
	return ret;
}
