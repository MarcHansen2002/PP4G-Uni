#pragma once
#include <numeric>
#include "Timer.h"

class BenchmarkTimer : public Timer
{
public:
	BenchmarkTimer(int counts)
	{
		index = 0;
		maxIndex = counts;
		times.reserve(maxIndex);
		times = std::vector<float>(maxIndex, 0);
	}

	float GetAverage()
	{
		float total = std::accumulate(times.begin(), times.end(), 0.f);
		return looped ? total / (float)maxIndex : total / (float)index;
	}
	int GetIndex() { return index; }

	float Restart() override;
	void Start();
	void Clear();

private:
	std::vector<float> times;
	int index;
	int maxIndex;
	bool looped = false;
};