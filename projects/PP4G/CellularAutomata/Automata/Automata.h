#pragma once

#include <algorithm>
#include "imgui.h"

#include <vulkan/vulkan.h>
#include "../../vulkan_resources.h"

constexpr static int GridSizeX = 256;
constexpr static int GridSizeY = 256;
constexpr static int CellCount = GridSizeX * GridSizeY;

enum Direction
{
	Up,
	Left,
	Right,
	Down
};

class Automata
{
public:

	vulkan_buffer CellBuffer;

	float Cells[CellCount]{};

	bool Alive(int x, int y)
	{
		int i = GetCellIndex(x, y);
		return (Cells[i] > 0);
	}
	int GetCellIndex(int x, int y)
	{
		return x + (y * GridSizeX);
	}

	void UpdateOnce()
	{
		// TODO: Cellular automata method
		float* newCells = new float[CellCount]{};

		for (int x = 0; x < GridSizeX; x++)
		{
			for (int y = 0; y < GridSizeY; y++)
			{
				int i = GetCellIndex(x, y);
				int aliveCount = 0;
				if (Alive(x - 1, y)) aliveCount++;
				if (Alive(x + 1, y)) aliveCount++;
				if (Alive(x - 1, y+1)) aliveCount++;
				if (Alive(x, y+1)) aliveCount++;
				if (Alive(x + 1, y+1)) aliveCount++;
				if (Alive(x - 1, y-1)) aliveCount++;
				if (Alive(x, y-1)) aliveCount++;
				if (Alive(x + 1, y-1)) aliveCount++;

				// Survive
				if (Alive(x, y) && (aliveCount == 2 || aliveCount == 3))
				{
					continue;
				}
				// Reproduce
				else if ((!Alive(x, y)) && aliveCount == 3)
				{
					newCells[i] = 1;
				}
				// Die
				else
				{
					newCells[i] = 0;
				}
			}
		}
		memcpy(Cells, newCells, sizeof(float) * CellCount);
		delete[] newCells;
	}

	void CreateBuffers(VkPhysicalDevice physDev, VkDevice dev)
	{
		if (!create_vulkan_buffer(physDev, dev,
			sizeof(float) * CellCount,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			CellBuffer))
		{
			assert(false);
			return;
		}
	}

	void MapBuffer(VkDevice dev)
	{
		if (!map_and_unmap_memory(
			dev,
			CellBuffer.memory, [this](void* mem)
			{
				memcpy(mem, Cells, sizeof(float) * CellCount);
			}
		))
		{
			return;
		}
	}
};