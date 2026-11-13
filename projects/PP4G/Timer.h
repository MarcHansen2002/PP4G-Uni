#pragma once
#include <chrono>
class Timer
{
public:
	Timer() { recorded = clock.now(); }

	virtual float Restart();
protected:
	std::chrono::high_resolution_clock clock;
	std::chrono::steady_clock::time_point recorded;
};