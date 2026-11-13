#pragma once
#include <numeric>
#include "Timer.h"

class BenchmarkTimer : public Timer
{
public:
	BenchmarkTimer(int counts)
	{
		maxIndex = counts;
		times.reserve(maxIndex);
	}

	float GetAverage()
	{
		float total = std::accumulate(times.begin(), times.end(), 0.f);
		return looped ? total / (float)maxIndex : total / (float)index;
	}

	float Restart() override;
	void Start();

private:
	std::vector<float> times;
	int index;
	int maxIndex;
	bool looped = false;
};