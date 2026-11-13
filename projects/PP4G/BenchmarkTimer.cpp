#include "BenchmarkTimer.h"
#include <algorithm>

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
void BenchmarkTimer::Clear()
{
	index = 0;
	times = std::vector<float>(maxIndex, 0);
	looped = false;
}