#pragma once

static constexpr int GridSizeX = 256;
static constexpr int GridSizeY = 256;
static constexpr int CellCount = GridSizeX * GridSizeY;

struct SmokeState
{
	float Density[CellCount];
	float VelocityX[CellCount];
	float VelocityY[CellCount];

	// TODO: Implement later
	//float Temperature[CellCount];
};

// TODO: Implement later on
struct SmokeSolver
{
	//float Pressure[CellCount];
	//float Divergence[CellCount];
	//float Vorticity[CellCount];
};