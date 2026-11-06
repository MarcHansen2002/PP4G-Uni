#ifndef SMOKE_PRESSURE_SOLVER
#define SMOKE_PRESSURE_SOLVER

struct PressureSolver
{
	float flowLeft;
	float flowRight;
	float flowTop;
	float flowBottom;
	int flowEdgeCount;
	uint isSolid;
	float velocityTerm;
};

#endif