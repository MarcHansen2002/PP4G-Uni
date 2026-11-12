#pragma once

#include <algorithm>
#include "imgui.h"

#include <vulkan/vulkan.h>
#include "../../vulkan_resources.h"

constexpr static int GridSizeX = 256;
constexpr static int GridSizeY = 256;
constexpr static int CellCount = GridSizeX * GridSizeY;

enum GameTypes
{
	E_GameOfLife,
};

class Automata
{
public:
	Automata() {};
	vulkan_buffer CellBuffer;
	float Cells[CellCount]{};
	virtual void UpdateOnce() {};
	virtual void Inspector() {};


	bool Alive(int x, int y)
	{
		int i = GetCellIndex(x, y);
		return (Cells[i] > 0);
	}
	int GetCellIndex(int x, int y)
	{
		return x + (y * GridSizeX);
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
			assert(false);
			return;
		}
	}
	void Terminate(VkDevice dev)
	{
		release_vulkan_buffer(dev, CellBuffer);
	}
private:
};