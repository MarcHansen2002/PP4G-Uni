#include "BenchmarkTimer.h"

float BenchmarkTimer::Restart()
{
	float delta = Timer::Restart();
	times[index] = delta;
	index++;

	if (index >= maxIndex)
	{
		looped = true;
		index = 0;
	}
	return delta;
}

void BenchmarkTimer::Start()
{
	recorded = clock.now();
}