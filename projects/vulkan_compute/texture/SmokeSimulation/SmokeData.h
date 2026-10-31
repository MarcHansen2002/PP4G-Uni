#pragma once
#include <algorithm>

static constexpr int GridSizeX = 256;
static constexpr int GridSizeY = 256;
static constexpr int CellCount = GridSizeX * GridSizeY;

static constexpr float FluidDensity = 1.f;
static constexpr float CellSize = 1.f;

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

struct SmokeState
{
	// Cell centered
	float Density[CellCount]{};
	float Temperature[CellCount]{};
	float Pressure[CellCount]{};
	// Edge Centered
	float VelocityX[CellCount + GridSizeX]{};
	float VelocityY[CellCount + GridSizeY]{};
	
	// Temporary values
	float TempVelocityX[CellCount + GridSizeX]{};
	float TempVelocityY[CellCount + GridSizeY]{};
	float TempDensity[CellCount]{};
	float TempTemperature[CellCount]{};

	PressureSolver PressureSolveData[CellCount]{};

	// TODO: Move all this to a compute shader
	int GetCellIndex(int x, int y)
	{
		return x + (y * GridSizeX);
	}
	int GetVelocityXIndex(int x, int y)
	{
		return x + y * (GridSizeX + 1);
	}
	int GetVelocityYIndex(int x, int y)
	{
		return x + y * GridSizeX;
	}

	bool IsSolid(int x, int y)
	{
		return x <= 0 || x >= GridSizeX - 1 ||
			y <= 0 || y >= GridSizeY - 1;
	}

	void PreparePressureSolver(float dTime)
	{
		for (int x = 0; x < GridSizeX; x++)
		{
			for (int y = 0; y < GridSizeY; y++)
			{
				int flowTop = !IsSolid(x, y + 1);
				int flowLeft = !IsSolid(x - 1, y);
				int flowRight = !IsSolid(x + 1, y);
				int flowBot = !IsSolid(x, y - 1);

				int edgeCount = flowTop + flowLeft + flowRight + flowBot;
				bool isSolid = IsSolid(x, y);

				float velTop = VelocityY[GetVelocityYIndex(x, y + 1)];
				float velLeft = VelocityX[GetVelocityXIndex(x, y)];
				float velRight = VelocityX[GetVelocityXIndex(x + 1, y)];
				float velBot = VelocityY[GetVelocityYIndex(x, y)];

				float velTerm = (velRight - velLeft + velTop - velBot) / dTime;

				PressureSolveData[GetCellIndex(x, y)] =
				{
					.flowLeft = (float)flowLeft,
					.flowRight = (float)flowRight,
					.flowTop = (float)flowTop,
					.flowBottom = (float)flowBot,
					.flowEdgeCount = edgeCount,
					.isSolid = isSolid,
					.velocityTerm = velTerm
				};
			}
		}
	}
	void PressureSolve(SmokeState* read)
	{
		for (int x = 0; x < GridSizeX; x++)
		{
			for (int y = 0; y < GridSizeY; y++)
			{
				float newPressure;
				PressureSolver info = PressureSolveData[GetCellIndex(x, y)];

				if (info.isSolid || info.flowEdgeCount == 0) newPressure = 0;
				else
				{
					float pressureTop = read->Pressure[GetCellIndex(x, (std::min)(y + 1, GridSizeY - 1))] * info.flowTop;
					float pressureLeft = read->Pressure[GetCellIndex((std::max)(x - 1, 0), y)] * info.flowLeft;
					float pressureRight = read->Pressure[GetCellIndex((std::min)(x + 1, GridSizeX - 1), y)] * info.flowRight;
					float pressureBottom = read->Pressure[GetCellIndex(x, (std::max)(y - 1, 0))] * info.flowBottom;

					float pressureSum = pressureRight + pressureLeft + pressureTop + pressureBottom;
					float CellSize = 1.f;

					newPressure = (pressureSum - FluidDensity * CellSize * info.velocityTerm) / (float)info.flowEdgeCount;
				}

				float oldPressure = read->Pressure[GetCellIndex(x, y)];
				float SOR = 1.f;
				read->Pressure[GetCellIndex(x, y)] = oldPressure + (newPressure - oldPressure) * SOR;
			}
		}
	}
	void RunPressureSolver(SmokeState* read, float dTime)
	{
		int numIts = 80;
		PreparePressureSolver(dTime);
		for (int i = 0; i < numIts; i++)
		{
			PressureSolve(read);
		}
	}

	void UpdateVelocities(SmokeState* state, float dTime)
	{
		float K = dTime / (FluidDensity * CellSize);

		// Horizontal velocities
		for (int x = 0; x < GridSizeX + 1; x++)
		{
			for (int y = 0; y < GridSizeY; y++)
			{
				if (IsSolid(x - 1, y) || IsSolid(x, y))
					continue;

				float pressureRight =
					state->Pressure[GetCellIndex(x, y)];

				float pressureLeft =
					state->Pressure[GetCellIndex(x - 1, y)];

				state->VelocityX[GetVelocityXIndex(x, y)]
					-= K * (pressureRight - pressureLeft);
			}
		}

		// Vertical velocities
		for (int x = 0; x < GridSizeX; x++)
		{
			for (int y = 0; y < GridSizeY + 1; y++)
			{
				if (IsSolid(x, y - 1) || IsSolid(x, y))
					continue;

				float pressureTop =
					state->Pressure[GetCellIndex(x, y)];

				float pressureBottom =
					state->Pressure[GetCellIndex(x, y - 1)];

				state->VelocityY[GetVelocityYIndex(x, y)]
					-= K * (pressureTop - pressureBottom);
			}
		}
	}

	float SampleBilinear(const float* data, int width, int height, float x, float y)
	{
		x = std::clamp(x, 0.0f, (float)(width - 1));
		y = std::clamp(y, 0.0f, (float)(height - 1));

		int x0 = (int)std::floor(x);
		int y0 = (int)std::floor(y);

		int x1 = (std::min)(x0 + 1, width - 1);
		int y1 = (std::min)(y0 + 1, height - 1);

		float tx = x - x0;
		float ty = y - y0;

		float bottom = std::lerp(
			data[x0 + y0 * width],
			data[x1 + y0 * width],
			tx
		);

		float top = std::lerp(
			data[x0 + y1 * width],
			data[x1 + y1 * width],
			tx
		);

		return std::lerp(bottom, top, ty);
	}
	glm::vec2 GetVelocityAtGridPos(SmokeState* state, glm::vec2 pos)
	{
		float velX = SampleBilinear(
			state->VelocityX,
			GridSizeX + 1,
			GridSizeY,
			pos.x,
			pos.y - 0.5f
		);

		float velY = SampleBilinear(
			state->VelocityY,
			GridSizeX,
			GridSizeY + 1,
			pos.x - 0.5f,
			pos.y
		);

		return glm::vec2(velX, velY);
	}

	void AdvectDye(SmokeState* state, float dTime)
	{
		float CellSize = 1.0f;

		for (int x = 0; x < GridSizeX; x++)
		{
			for (int y = 0; y < GridSizeY; y++)
			{
				int i = GetCellIndex(x, y);

				if (IsSolid(x, y))
				{
					TempDensity[i] = 0.0f;
					continue;
				}

				glm::vec2 pos(
					x + 0.5f,
					y + 0.5f
				);

				glm::vec2 vel = GetVelocityAtGridPos(state, pos);

				glm::vec2 posPrev =
					pos - vel * (dTime / CellSize);

				TempDensity[i] = SampleBilinear(
					state->Density,
					GridSizeX,
					GridSizeY,
					posPrev.x - 0.5f,
					posPrev.y - 0.5f
				);
			}
		}

		// Copy temporary density back
		for (int i = 0; i < GridSizeX * GridSizeY; i++)
		{
			state->Density[i] = TempDensity[i];
		}
	}
	float SampleVelocityX(SmokeState* state, glm::vec2 pos)
	{
		return SampleBilinear(
			state->VelocityX,
			GridSizeX + 1,
			GridSizeY,
			pos.x,
			pos.y - 0.5f
		);
	}

	float SampleVelocityY(SmokeState* state, glm::vec2 pos)
	{
		return SampleBilinear(
			state->VelocityY,
			GridSizeX,
			GridSizeY + 1,
			pos.x - 0.5f,
			pos.y
		);
	}
	void AdvectVelocity(SmokeState* state, float dTime)
	{
		float CellSize = 1.0f;

		for (int x = 0; x < GridSizeX + 1; x++)
		{
			for (int y = 0; y < GridSizeY; y++)
			{
				int i = GetVelocityXIndex(x, y);

				if (IsSolid(x - 1, y) || IsSolid(x, y))
				{
					TempVelocityX[i] = 0.0f;
					continue;
				}

				glm::vec2 pos(
					(float)x,
					y + 0.5f
				);

				glm::vec2 vel = GetVelocityAtGridPos(state, pos);

				glm::vec2 posPrev =
					pos - vel * (dTime / CellSize);

				TempVelocityX[i] = SampleVelocityX(state, posPrev);
			}
		}

		for (int x = 0; x < GridSizeX; x++)
		{
			for (int y = 0; y < GridSizeY + 1; y++)
			{
				int i = GetVelocityYIndex(x, y);

				if (IsSolid(x, y - 1) || IsSolid(x, y))
				{
					TempVelocityY[i] = 0.0f;
					continue;
				}

				glm::vec2 pos(
					x + 0.5f,
					(float)y
				);

				glm::vec2 vel = GetVelocityAtGridPos(state, pos);

				glm::vec2 posPrev =
					pos - vel * (dTime / CellSize);

				TempVelocityY[i] = SampleVelocityY(state, posPrev);
			}
		}

		for (int i = 0; i < (GridSizeX + 1) * GridSizeY; i++)
		{
			state->VelocityX[i] = TempVelocityX[i];
		}

		for (int i = 0; i < GridSizeX * (GridSizeY + 1); i++)
		{
			state->VelocityY[i] = TempVelocityY[i];
		}
	}
};

