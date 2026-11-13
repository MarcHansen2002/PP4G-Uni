#pragma once
#include <algorithm>
#include <string>
#include "imgui.h"

#include <vulkan/vulkan.h>
#include "../../vulkan_resources.h"

static constexpr int GridSizeX = 256;
static constexpr int GridSizeY = 256;
static constexpr int CellCount = GridSizeX * GridSizeY;

static constexpr int VeloSizeX = (GridSizeX + 1) * GridSizeY;
static constexpr int VeloSizeY = GridSizeX * (GridSizeY + 1);

static constexpr float CellSize = 1.0f;

struct smoke_push_constants_compute
{
	u32 GridSizeX;
	u32 GridSizeY;
	float dTime;
	float CellSize;
};

struct PressureSolver
{
	float flowLeft;
	float flowRight;
	float flowTop;
	float flowBottom;
	int32_t flowEdgeCount;
	uint32_t isSolid;
	float velocityTerm;
};

struct SmokeCell
{
	SmokeCell()
	{
		Density = 0.f;
		Red = 0.f;
		Blue = 0.f;
		Green = 0.f;
	}
	
	float Red;
	float Green;
	float Blue;
	float Density;
};

struct SmokeState
{
	// Cell centered
	//float Density[CellCount]{};
	SmokeCell Density[CellCount]{};
	float Temperature[CellCount]{};
	float Pressure[CellCount]{};
	float PressureB[CellCount]{};
	// Edge Centered
	float VelocityX[CellCount + GridSizeX]{};
	float VelocityY[CellCount + GridSizeY]{};
	
	// Temporary values
	float TempVelocityX[CellCount + GridSizeX]{};
	float TempVelocityY[CellCount + GridSizeY]{};
	//float TempDensity[CellCount]{};
	SmokeCell TempDensity[CellCount]{};
	float TempTemperature[CellCount]{};

	PressureSolver PressureSolveData[CellCount]{};

	vulkan_buffer Buffer_Density;
	vulkan_buffer Buffer_Temperature;
	vulkan_buffer Buffer_Pressure;
	vulkan_buffer Buffer_PressureB;
	vulkan_buffer Buffer_VelocityX;
	vulkan_buffer Buffer_VelocityY;
	vulkan_buffer Buffer_Temp_VelocityX;
	vulkan_buffer Buffer_Temp_VelocityY;
	vulkan_buffer Buffer_Temp_Density;
	vulkan_buffer Buffer_Temp_Temperature;

	vulkan_buffer Buffer_PressureSolve;

	void ClearSmoke()
	{
		std::fill_n(Density, CellCount, SmokeCell());
		std::fill_n(Temperature, CellCount, 0.f);
		std::fill_n(Pressure, CellCount, 0.f);
		std::fill_n(VelocityX, VeloSizeX, 0.f);
		std::fill_n(VelocityY, VeloSizeY, 0.f);
		std::fill_n(TempVelocityX, VeloSizeX, 0.f);
		std::fill_n(TempVelocityY, VeloSizeY, 0.f);
	}

	int CreateBuffers(VkPhysicalDevice physDev, VkDevice dev)
	{
		if (!create_vulkan_buffer(physDev, dev,
			sizeof(SmokeCell) * CellCount,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			Buffer_Density))
		{
			assert(false);
			return -1;
		}
		if (!create_vulkan_buffer(physDev, dev,
			sizeof(float) * CellCount,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			Buffer_Temperature))
		{
			assert(false);
			return -1;
		}
		if (!create_vulkan_buffer(physDev, dev,
			sizeof(float) * CellCount,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			Buffer_Pressure))
		{
			assert(false);
			return -1;
		}
		if (!create_vulkan_buffer(physDev, dev,
			sizeof(float) * CellCount,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			Buffer_PressureB))
		{
			assert(false);
			return -1;
		}
		if (!create_vulkan_buffer(physDev, dev,
			sizeof(float) * VeloSizeX,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			Buffer_VelocityX))
		{
			assert(false);
			return -1;
		}
		if (!create_vulkan_buffer(physDev, dev,
			sizeof(float) * VeloSizeY,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			Buffer_VelocityY))
		{
			assert(false);
			return -1;
		}
		if (!create_vulkan_buffer(physDev, dev,
			sizeof(float) * VeloSizeX,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			Buffer_Temp_VelocityX))
		{
			assert(false);
			return -1;
		}
		if (!create_vulkan_buffer(physDev, dev,
			sizeof(float) * VeloSizeY,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			Buffer_Temp_VelocityY))
		{
			assert(false);
			return -1;
		}
		if (!create_vulkan_buffer(physDev, dev,
			sizeof(SmokeCell) * CellCount,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			Buffer_Temp_Density))
		{
			assert(false);
			return -1;
		}
		if (!create_vulkan_buffer(physDev, dev,
			sizeof(float) * CellCount,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			Buffer_Temp_Temperature))
		{
			assert(false);
			return -1;
		}
		if (!create_vulkan_buffer(physDev, dev,
			sizeof(PressureSolver) * CellCount,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			Buffer_PressureSolve))
		{
			assert(false);
			return -1;
		}
	}

	int MapBuffers(VkDevice dev)
	{
		if (!map_and_unmap_memory(
			dev,
			Buffer_VelocityX.memory, [this](void* mem)
			{
				memcpy(mem, VelocityX, sizeof(float) * VeloSizeX);
			}
		))
		{
			return 0;
		}
		if (!map_and_unmap_memory(
			dev,
			Buffer_VelocityY.memory, [this](void* mem)
			{
				memcpy(mem, VelocityY, sizeof(float) * VeloSizeY);
			}
		))
		{
			return 0;
		}

		if (!map_and_unmap_memory(
			dev,
			Buffer_Density.memory, [this](void* mem)
			{
				memcpy(mem, Density, sizeof(SmokeCell) * CellCount);
			}
		))
		{
			return 0;
		}

		if (!map_and_unmap_memory(
			dev,
			Buffer_Pressure.memory, [this](void* mem)
			{
				memcpy(mem, Pressure, sizeof(float) * CellCount);
			}))
		{
			return 0;
		}

		return 1;
	}

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
	// Helper Maths
	glm::vec2 GetVelocityAtGridPos(glm::vec2 pos)
	{
		float velX = SampleBilinear(
			VelocityX,
			GridSizeX + 1,
			GridSizeY,
			pos.x,
			pos.y - 0.5f
		);

		float velY = SampleBilinear(
			VelocityY,
			GridSizeX,
			GridSizeY + 1,
			pos.x - 0.5f,
			pos.y
		);

		return glm::vec2(velX, velY);
	}
	float SampleVelocityX(glm::vec2 pos)
	{
		return SampleBilinear(
			VelocityX,
			GridSizeX + 1,
			GridSizeY,
			pos.x,
			pos.y - 0.5f
		);
	}
	float SampleVelocityY(glm::vec2 pos)
	{
		return SampleBilinear(
			VelocityY,
			GridSizeX,
			GridSizeY + 1,
			pos.x - 0.5f,
			pos.y
		);
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
	void PressureSolve(float FluidDensity, int iter)
	{
		for (int x = 0; x < GridSizeX; x++)
		{
			for (int y = 0; y < GridSizeY; y++)
			{
				float newPressure;

				float* a = (iter % 2 == 0) ? Pressure : PressureB;
				float* b = (iter % 2 == 0) ? PressureB : Pressure;

				PressureSolver info = PressureSolveData[GetCellIndex(x, y)];

				if (info.isSolid || info.flowEdgeCount == 0) newPressure = 0;
				else
				{
					float pressureTop = a[GetCellIndex(x, (std::min)(y + 1, GridSizeY - 1))] * info.flowTop;
					float pressureLeft = a[GetCellIndex((std::max)(x - 1, 0), y)] * info.flowLeft;
					float pressureRight = a[GetCellIndex((std::min)(x + 1, GridSizeX - 1), y)] * info.flowRight;
					float pressureBottom = a[GetCellIndex(x, (std::max)(y - 1, 0))] * info.flowBottom;

					float pressureSum = pressureRight + pressureLeft + pressureTop + pressureBottom;
					float CellSize = 1.f;

					newPressure = (pressureSum - FluidDensity * CellSize * info.velocityTerm) / (float)info.flowEdgeCount;
				}

				float oldPressure = a[GetCellIndex(x, y)];
				float SOR = 1.f;
				b[GetCellIndex(x, y)] = oldPressure + (newPressure - oldPressure) * SOR;
			}
		}
	}
	void RunPressureSolver(float FluidDensity, float dTime)
	{
		int numIts = 80;
		PreparePressureSolver(dTime);
		for (int i = 0; i < numIts; i++)
		{
			PressureSolve(FluidDensity, i);
		}
	}

	void UpdateVelocities(float FluidDensity, float dTime)
	{
		float K = dTime / (FluidDensity * CellSize);

		// Horizontal velocities
		for (int x = 0; x < GridSizeX + 1; x++)
		{
			for (int y = 0; y < GridSizeY; y++)
			{
				if (IsSolid(x - 1, y) || IsSolid(x, y))
					continue;

				float pressureRight = Pressure[GetCellIndex(x, y)];
				float pressureLeft = Pressure[GetCellIndex(x - 1, y)];
				VelocityX[GetVelocityXIndex(x, y)] -= K * (pressureRight - pressureLeft);
			}
		}

		// Vertical velocities
		for (int x = 0; x < GridSizeX; x++)
		{
			for (int y = 0; y < GridSizeY + 1; y++)
			{
				if (IsSolid(x, y - 1) || IsSolid(x, y))
					continue;

				float pressureTop = Pressure[GetCellIndex(x, y)];
				float pressureBottom = Pressure[GetCellIndex(x, y - 1)];
				VelocityY[GetVelocityYIndex(x, y)] -= K * (pressureTop - pressureBottom);
			}
		}
	}

	void AdvectDye(float dTime)
	{
		//for (int x = 0; x < GridSizeX; x++)
		//{
		//	for (int y = 0; y < GridSizeY; y++)
		//	{
		//		int i = GetCellIndex(x, y);
		//
		//		if (IsSolid(x, y))
		//		{
		//			TempDensity[i] = 0.0f;
		//			continue;
		//		}
		//
		//		glm::vec2 pos(
		//			x + 0.5f,
		//			y + 0.5f
		//		);
		//
		//		glm::vec2 vel = GetVelocityAtGridPos(pos);
		//
		//		glm::vec2 posPrev =
		//			pos - vel * (dTime / CellSize);
		//
		//		TempDensity[i] = SampleBilinear(
		//			Density,
		//			GridSizeX,
		//			GridSizeY,
		//			posPrev.x - 0.5f,
		//			posPrev.y - 0.5f
		//		);
		//	}
		//}

		// Copy temporary density back
		for (int i = 0; i < GridSizeX * GridSizeY; i++)
		{
			Density[i] = TempDensity[i];
		}
	}

	void AdvectVelocity(float dTime)
	{
		// Horizontal
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

				glm::vec2 vel = GetVelocityAtGridPos(pos);

				glm::vec2 posPrev =
					pos - vel * (dTime / CellSize);

				TempVelocityX[i] = SampleVelocityX(posPrev);
			}
		}
		// Vertical
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

				glm::vec2 vel = GetVelocityAtGridPos(pos);

				glm::vec2 posPrev =
					pos - vel * (dTime / CellSize);

				TempVelocityY[i] = SampleVelocityY(posPrev);
			}
		}

		for (int i = 0; i < (GridSizeX + 1) * GridSizeY; i++)
		{
			VelocityX[i] = TempVelocityX[i];
		}

		for (int i = 0; i < GridSizeX * (GridSizeY + 1); i++)
		{
			VelocityY[i] = TempVelocityY[i];
		}
	}
};

struct SmokeEmitter
{
	float speed = 3.f;

	int x = GridSizeX / 2;
	int y = GridSizeY / 2;

	float r = 1.f;
	float g = 1.f;
	float b = 1.f;

	float Density = 2.f;
	float Temperature = 5.f;

	float VelocityX = 0;
	float VelocityY = -10;

	bool active = false;

	void Emit(SmokeState& state, float dTime)
	{
		if (!active) return;

		timer += dTime;
		if (timer >= speed * 1000)
		{
			timer = 0.f;

			int i = state.GetCellIndex(x, y);

			state.Density[i].Density = Density;
			state.Density[i].Red = r;
			state.Density[i].Green = g;
			state.Density[i].Blue = b;
			state.Temperature[i] = Temperature;

			state.VelocityX[state.GetVelocityXIndex(x, y)] = VelocityX;
			state.VelocityX[state.GetVelocityXIndex(x + 1, y)] = VelocityX;

			state.VelocityY[state.GetVelocityYIndex(x, y)] = VelocityY;
			state.VelocityY[state.GetVelocityYIndex(x, y + 1)] = VelocityY;
		}
	}

	void ShowInspector(int index = 0)
	{
		std::string title = "Emitter - " + std::to_string(index);
		if (ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Checkbox("Enabled", &active);
			ImGui::SliderFloat("Spawn every X second", &speed, 0.001f, 10.f);

			ImGui::SliderInt("X Spawn", &x, 0, GridSizeX - 1);
			ImGui::SliderInt("Y Spawn", &y, 0, GridSizeY - 1);

			ImGui::SliderFloat("Density", &Density, 0.f, 10.f);
			ImGui::SliderFloat("Temperature", &Temperature, -50.f, 50.f);

			ImGui::SliderFloat("Velocity X", &VelocityX, -10.f, 10.f);
			ImGui::SliderFloat("Velocity Y", &VelocityY, -10.f, 10.f);

			ImGui::SliderFloat("Red", &r, 0.f, 1.f);
			ImGui::SliderFloat("Green", &g, 0.f, 1.f);
			ImGui::SliderFloat("Blue", &b, 0.f, 1.f);
		}
	}

private:
	float timer = 0.f;
};