# include "GlobalFunctions.h"

GlobalTime Time;

uint64_t GenerateUUID()
{
	const unsigned int bytes = sizeof(long long);
	char myString[bytes];
	uint64_t ret;

	for (int n = 0; n < bytes; n++)
	{
		myString[n] = rand() % 255;
	}

	memcpy(&ret, myString, bytes);
	return ret;
}