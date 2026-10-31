#pragma once
#include <algorithm>

static constexpr int GridSizeX = 256;
static constexpr int GridSizeY = 256;
static constexpr int CellCount = GridSizeX * GridSizeY;

struct SmokeState
{
	// Cell centered
	float Density[CellCount]{};
	float Temperature[CellCount]{};
	float Pressure[CellCount]{};
	// Edge Centered
	float VelocityX[CellCount + GridSizeX]{};
	float VelocityY[CellCount + GridSizeY]{};
	
	
};

struct PressureSolver
{
	float flowLeft;
	float flowRight;
	float flowTop;
	float flowBottom;
	int flowEdgeCount;
	bool isSolid;
	float velocityTerm;
};