#include "Timer.h"

float Timer::Restart()
{
	std::chrono::steady_clock::time_point end = clock.now();
	float delta = std::chrono::duration_cast<std::chrono::milliseconds>(end - recorded).count();
	recorded = end;

	return delta;
}